#include "TelemetryParser.h"
#include "TelemetryTypes.h"
#include "Heatmap.h"
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <mutex>
#include <map>

std::vector<std::string> split(const std::string& s, char d) {
    std::vector<std::string> r;
    std::stringstream ss(s);
    std::string t;
    while (std::getline(ss, t, d)) 
        if (!t.empty()) r.push_back(t);
    return r;
}

int extractInt(const std::string& str, const std::string& param) {
    size_t pos = str.find(param + "=");
    if (pos == std::string::npos) return -999;
    
    size_t start = pos + param.length() + 1;
    size_t end = start;
    while (end < str.length() && str[end] != '|' && str[end] != ' ') end++;
    return std::stoi(str.substr(start, end - start));
}

float parseTimeToSeconds(const std::string& timeStr) {
    std::tm tm = {};
    std::istringstream ss(timeStr);
    ss >> std::get_time(&tm, "%H:%M:%S");
    return tm.tm_hour * 3600.0f + tm.tm_min * 60.0f + tm.tm_sec;
}

TelemetryPacket parsePacket(const std::string& msg, float elapsed) {
    TelemetryPacket packet;
    packet.elapsed_time = elapsed;
    auto parts = split(msg, '|');
    if (parts.size() < 5) return packet;
    
    packet.timestamp = parts[0];
    packet.lat = std::stod(parts[1]);
    packet.lon = std::stod(parts[2]);
    packet.alt = std::stod(parts[3]);
    packet.accuracy = std::stod(parts[4]);
    
    if (parts.size() > 5) {
        CellData cell;
        cell.type = parts[5];
        if (cell.type == "LTE") {
            cell.pci = extractInt(msg, "pci");
            cell.rsrp = extractInt(msg, "rsrp");
            cell.rsrq = extractInt(msg, "rsrq");
            cell.rssi = extractInt(msg, "rssi");
            cell.sinr = extractInt(msg, "sinr");
        } else if (cell.type == "5G") {
            cell.pci = extractInt(msg, "pci");
            cell.ssRsrp = extractInt(msg, "ssRsrp");
        } else if (cell.type == "GSM" || cell.type == "WCDMA") {
            cell.pci = extractInt(msg, "cid");
            cell.dbm = extractInt(msg, "dbm");
        }
        packet.cells.push_back(cell);
    }
    return packet;
}

void TelemetrySession::addPacket(const TelemetryPacket& packet) {
    std::lock_guard<std::mutex> lock(mtx);
    current_packet = packet;
    time_history.push_back(packet.elapsed_time);
    lat_values.push_back(packet.lat);
    lon_values.push_back(packet.lon);
    
    for (const auto& cell : packet.cells) {
        int signal = (cell.type == "LTE") ? cell.rsrp : 
                     (cell.type == "5G") ? cell.ssRsrp : cell.dbm;
        if (pci_signal_history.find(cell.pci) == pci_signal_history.end()) {
            pci_signal_history[cell.pci] = std::vector<float>();
            pci_types[cell.pci] = cell.type;
        }
        pci_signal_history[cell.pci].push_back(signal);
    }
    
    heat_add_points(packet);
}

void TelemetrySession::clear() {
    std::lock_guard<std::mutex> lock(mtx);
    time_history.clear();
    lat_values.clear();
    lon_values.clear();
    pci_signal_history.clear();
    pci_types.clear();
}