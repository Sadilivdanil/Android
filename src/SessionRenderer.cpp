#include "SessionRenderer.h"
#include "TelemetryParser.h"
#include "Mercator.h"
#include "TileManager.h"
#include "Heatmap.h"
#include "Constants.h"
#include <imgui.h>
#include <implot.h>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <mutex>
#include <cmath>
#include <set>
#include <map>

static bool g_showHeatmap = false;
static int g_heatmapCriterionIndex = 0;
static const char* g_criterionNames[] = {"RSRP", "RSRQ", "RSSI", "Altitude"};
static std::set<std::string> g_requestedTiles;

void load_history_tab(const std::string& filename, 
                      std::vector<TelemetrySession*>& sessions, 
                      std::mutex& mtx) {
    std::ifstream f(filename);
    if (!f.is_open()) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return;
    }
    
    std::vector<std::pair<float, std::string>> packets;
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        auto parts = split(line, '|');
        if (parts.size() >= 1) {
            packets.push_back({parseTimeToSeconds(parts[0]), line});
        }
    }
    f.close();
    
    if (packets.empty()) {
        std::cerr << "No valid packets found in file!" << std::endl;
        return;
    }
    
    std::sort(packets.begin(), packets.end());
    
    TelemetrySession* ns = new TelemetrySession();
    ns->name = "History_" + std::to_string(sessions.size());
    float first = packets[0].first;
    for (const auto& p : packets) {
        ns->addPacket(parsePacket(p.second, p.first - first));
    }
    
    std::cout << "Loaded " << packets.size() << " packets into session: " << ns->name << std::endl;
    
    std::lock_guard<std::mutex> lock(mtx);
    sessions.push_back(ns);
}

void render_session_contents(TelemetrySession* s) {
    std::lock_guard<std::mutex> lock(s->mtx);
    
    if (ImGui::BeginTabBar("SessionTabs")) {
        if (ImGui::BeginTabItem("Telemetry Data")) {
            ImGui::Columns(2, nullptr, true);
            ImGui::TextColored(ImVec4(0,1,0,1), "Last Sync: %s", s->current_packet.timestamp.c_str());
            ImGui::Separator();
            ImGui::Text("GPS: %.6f, %.6f", s->current_packet.lat, s->current_packet.lon);
            ImGui::Text("Alt: %.1f m | Acc: %.1f m", s->current_packet.alt, s->current_packet.accuracy);
            ImGui::Separator();
            ImGui::Text("Total GPS Points: %d", (int)s->lat_values.size());
            ImGui::Text("Detected Cells (%d):", (int)s->current_packet.cells.size());
            for (const auto& cell : s->current_packet.cells) {
                std::string d = cell.type;
                if (cell.type == "LTE") {
                    d += " | PCI:" + std::to_string(cell.pci) + " | RSRP:" + std::to_string(cell.rsrp) + "dBm";
                } else if (cell.type == "5G") {
                    d += " | PCI:" + std::to_string(cell.pci) + " | SS-RSRP:" + std::to_string(cell.ssRsrp) + "dBm";
                } else {
                    d += " | PCI:" + std::to_string(cell.pci) + " | Level:" + std::to_string(cell.dbm) + "dBm";
                }
                ImGui::BulletText("%s", d.c_str());
            }
            
            ImGui::NextColumn();
            if (ImPlot::BeginPlot("Movement Path", ImVec2(-1, 300), ImPlotFlags_Equal)) {
                ImPlot::SetupAxes("Longitude", "Latitude");
                if (!s->lat_values.empty()) {
                    ImPlot::PlotLine("Route", s->lon_values.data(), s->lat_values.data(), (int)s->lat_values.size());
                    double clat = s->current_packet.lat, clon = s->current_packet.lon;
                    ImPlot::PlotScatter("Current Pos", &clon, &clat, 1);
                }
                ImPlot::EndPlot();
            }
            
            ImGui::Columns(1);
            ImGui::Separator();
            
            if (ImPlot::BeginPlot("Signal Analysis (dBm)", ImVec2(-1, 400))) {
                ImPlot::SetupAxes("Time (seconds)", "Level (dBm)");
                ImPlot::SetupAxisLimits(ImAxis_Y1, -120, -40, ImPlotCond_Once);
                
                if (s->pci_signal_history.empty()) {
                    ImPlot::PlotText("No signal data available", 5, -80);
                }
                
                for (const auto& [pci, hist] : s->pci_signal_history) {
                    if (!hist.empty() && !s->time_history.empty()) {
                        size_t plotSize = std::min(hist.size(), s->time_history.size());
                        if (plotSize > 0) {
                            std::string label = s->pci_types[pci] + " (PCI:" + std::to_string(pci) + ")";
                            ImPlot::PlotLine(label.c_str(), 
                                            s->time_history.data(), 
                                            hist.data(), 
                                            (int)plotSize);
                        }
                    }
                }
                ImPlot::EndPlot();
            }
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("OSM Map")) {
            static int current_zoom = DEFAULT_ZOOM;
            static double center_lat = NOVOSIBIRSK_LAT;
            static double center_lon = NOVOSIBIRSK_LON;
            static bool double_click_pending = false;
            static double pending_lat = 0;
            static double pending_lon = 0;
            
            static const ImPlotAxisFlags flags = ImPlotAxisFlags_NoLabel | ImPlotAxisFlags_NoGridLines |
                                                  ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoTickLabels |
                                                  ImPlotAxisFlags_NoInitialFit | ImPlotAxisFlags_NoMenus;
            
            ImGui::Text("Zoom: %d", current_zoom);
            ImGui::SameLine();
            if (ImGui::Button("-##MapZoom")) {
                current_zoom = std::max(MIN_ZOOM, current_zoom - 1);
                g_requestedTiles.clear();
            }
            ImGui::SameLine();
            if (ImGui::Button("+##MapZoom")) {
                current_zoom = std::min(MAX_ZOOM, current_zoom + 1);
                g_requestedTiles.clear();
            }
            ImGui::SameLine();
            if (ImGui::Button("Center on GPS##MapCenter")) {
                if (s->current_packet.lat != 0 || s->current_packet.lon != 0) {
                    center_lat = s->current_packet.lat;
                    center_lon = s->current_packet.lon;
                    g_requestedTiles.clear();
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Novosibirsk##MapCenterNS")) {
                center_lat = NOVOSIBIRSK_LAT;
                center_lon = NOVOSIBIRSK_LON;
                g_requestedTiles.clear();
            }
            
            ImGui::Separator();
            
            ImGui::Checkbox("Show Heatmap", &g_showHeatmap);
            if (g_showHeatmap) {
                ImGui::SameLine();
                int currentMetric = (int)g_heat_metric;
                if (ImGui::Combo("Metric", &currentMetric, "RSRP\0RSRQ\0RSSI\0Altitude\0")) {
                    g_heat_metric = (HeatMetric)currentMetric;
                    heat_invalidate_cache();
                    g_requestedTiles.clear();
                }
                
                ImGui::InputInt("EARFCN Filter", &g_heat_earfcn_filter);
                ImGui::InputInt("PCI Filter", &g_heat_pci_filter);
                
                if (ImGui::Button("Apply Filters")) {
                    heat_invalidate_cache();
                    g_requestedTiles.clear();
                }
                
                ImGui::SliderFloat("Radius (m)", &g_idw_radius_m, 10.0f, 40.0f);
                ImGui::SliderFloat("IDW Power", &g_idw_power, 1.0f, 3.0f);
                
                if (ImGui::Button("Refresh Heatmap")) {
                    heat_invalidate_cache();
                    g_requestedTiles.clear();
                }
                
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), 
                                   "Points: %d", (int)g_heat_points.size());
            }
            
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), 
                               "Hint: Double-click to zoom, Right-click + drag to pan");
            
            ImVec2 map_size = ImGui::GetContentRegionAvail();
            if (map_size.x > 100 && map_size.y > 100) {
                double min_lon, max_lon, min_mercator_y, max_mercator_y;
                double map_ratio = map_size.x / map_size.y;
                computeVisibleArea(center_lon, center_lat, current_zoom, map_ratio,
                                   min_lon, max_lon, min_mercator_y, max_mercator_y);
                
                if (ImPlot::BeginPlot("##MapPlot", map_size, ImPlotFlags_NoLegend | ImPlotFlags_NoMenus)) {
                    ImPlot::SetupAxis(ImAxis_X1, nullptr, flags);
                    ImPlot::SetupAxis(ImAxis_Y1, nullptr, flags);
                    ImPlot::SetupAxisLimits(ImAxis_X1, min_lon, max_lon, ImPlotCond_Always);
                    ImPlot::SetupAxisLimits(ImAxis_Y1, min_mercator_y, max_mercator_y, ImPlotCond_Always);
                    
                    ImPlotRect limits = ImPlot::GetPlotLimits();
                    
                    if (ImPlot::IsPlotHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                        ImVec2 mouse_pos = ImGui::GetMousePos();
                        ImVec2 plot_pos = ImPlot::GetPlotPos();
                        ImVec2 plot_size = ImPlot::GetPlotSize();
                        
                        double mouse_x_norm = (mouse_pos.x - plot_pos.x) / plot_size.x;
                        double mouse_y_norm = (mouse_pos.y - plot_pos.y) / plot_size.y;
                        
                        double click_lon = limits.X.Min + mouse_x_norm * (limits.X.Max - limits.X.Min);
                        double click_mercator_y = limits.Y.Min + (1.0 - mouse_y_norm) * (limits.Y.Max - limits.Y.Min);
                        double click_lat = MercatorYToLat(click_mercator_y);
                        
                        double_click_pending = true;
                        pending_lat = click_lat;
                        pending_lon = click_lon;
                    }
                    
                    int minX = static_cast<int>(std::floor(MercatorXToTileX(limits.X.Min, current_zoom)));
                    int minY = static_cast<int>(std::floor(MercatorYToTileY(limits.Y.Max, current_zoom)));
                    int maxX = static_cast<int>(std::floor(MercatorXToTileX(limits.X.Max, current_zoom)));
                    int maxY = static_cast<int>(std::floor(MercatorYToTileY(limits.Y.Min, current_zoom)));
                    
                    int maxTileCount = (1 << current_zoom) - 1;
                    minX = std::max(0, minX);
                    maxX = std::min(maxTileCount, maxX);
                    minY = std::max(0, minY);
                    maxY = std::min(maxTileCount, maxY);
                    
                    for (int x = minX; x <= maxX; x++) {
                        for (int y = minY; y <= maxY; y++) {
                            std::string tileId = std::to_string(current_zoom) + "/" + std::to_string(x) + "/" + std::to_string(y);
                            
                            GLuint textureId = 0;
                            if (getTileTexture(tileId, textureId)) {
                                ImPlotPoint minPoint(TileXToMercatorX(x, current_zoom), TileYToMercatorY(y + 1, current_zoom));
                                ImPlotPoint maxPoint(TileXToMercatorX(x + 1, current_zoom), TileYToMercatorY(y, current_zoom));
                                ImPlot::PlotImage(("##tile_" + tileId).c_str(), (ImTextureID)(intptr_t)textureId, minPoint, maxPoint);
                            } else {
                                requestTile(tileId, current_zoom, x, y);
                            }

                            HeatTile heatTile = heat_get_tile(current_zoom, x, y);
                            if (heatTile.loaded && heatTile.tex_id != 0) {
                                ImPlotPoint heatMin(TileXToMercatorX(x, current_zoom), TileYToMercatorY(y + 1, current_zoom));
                                ImPlotPoint heatMax(TileXToMercatorX(x + 1, current_zoom), TileYToMercatorY(y, current_zoom));
                                ImPlot::PlotImage(("##heat_" + tileId).c_str(), (ImTextureID)(intptr_t)heatTile.tex_id, heatMin, heatMax);
                            } else {
                                heat_request_tile(current_zoom, x, y);
                            }
                        }
                    }
                    
                    if (!s->lat_values.empty()) {
                        std::vector<double> mercator_y;
                        for (float lat : s->lat_values) {
                            mercator_y.push_back(LatToMercatorY(lat));
                        }
                        std::vector<double> lons(s->lon_values.begin(), s->lon_values.end());
                        ImPlot::PlotLine("Path", lons.data(), mercator_y.data(), (int)mercator_y.size());
                    }
                    
                    if (s->current_packet.lat != 0 || s->current_packet.lon != 0) {
                        double curr_mercator_y = LatToMercatorY(s->current_packet.lat);
                        double curr_lon = s->current_packet.lon;
                        ImPlot::PlotScatter("Current", &curr_lon, &curr_mercator_y, 1);
                    }
                    
                    if (ImPlot::IsPlotHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
                        ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
                        if (drag_delta.x != 0 || drag_delta.y != 0) {
                            ImPlotRect newLimits = ImPlot::GetPlotLimits();
                            double drag_x = drag_delta.x * (newLimits.X.Max - newLimits.X.Min) / map_size.x;
                            double drag_y = drag_delta.y * (newLimits.Y.Max - newLimits.Y.Min) / map_size.y;
                            center_lon = (newLimits.X.Min + newLimits.X.Max) / 2.0 - drag_x;
                            double center_merc = (newLimits.Y.Min + newLimits.Y.Max) / 2.0 - drag_y;
                            center_lat = MercatorYToLat(center_merc);
                            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
                            g_requestedTiles.clear();
                        }
                    }
                    
                    ImPlot::EndPlot();
                    
                    if (double_click_pending) {
                        current_zoom = std::min(MAX_ZOOM, current_zoom + 1);
                        center_lon = pending_lon;
                        center_lat = pending_lat;
                        double_click_pending = false;
                        g_requestedTiles.clear();
                    }
                }
            }
            
            ImGui::EndTabItem();
        }
        
        ImGui::EndTabBar();
    }
}