#include "GuiManager.h"
#include "Constants.h"
#include "TelemetryTypes.h"
#include "SessionRenderer.h"
#include "TileManager.h"
#include "Heatmap.h"
#include <SDL.h>
#include <GL/glew.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>
#include <thread>
#include <iostream>
#include <vector>
#include <mutex>

extern std::vector<TelemetrySession*> all_sessions;
extern std::mutex global_sessions_mtx;

void run_gui() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Telemetry Server", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1400, 900,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_GLContext gl = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl);
    glewInit();
    
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGui_ImplSDL2_InitForOpenGL(window, gl);
    ImGui_ImplOpenGL3_Init("#version 130");
    
    std::thread tileWorker(FetchWorker);
    std::thread heatWorker(heat_worker_thread);
    heatWorker.detach();
    
    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            ImGui_ImplSDL2_ProcessEvent(&e);
            if (e.type == SDL_QUIT) running = false;
        }
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        
        ImGui::Begin("Global Controls");
        if (ImGui::Button("IMPORT LOG FILE", ImVec2(-1, 40))) {
            load_history_tab("log.json", all_sessions, global_sessions_mtx);
        }
        if (ImGui::Button("Clear Live History", ImVec2(-1, 30))) {
            std::lock_guard<std::mutex> lock(global_sessions_mtx);
            if (!all_sessions.empty() && all_sessions[0] != nullptr) {
                all_sessions[0]->clear();
                heat_invalidate_cache();
            }
        }
        ImGui::End();
        
        ImGui::Begin("Data View");
        if (ImGui::BeginTabBar("SessionsTabBar")) {
            std::lock_guard<std::mutex> lock(global_sessions_mtx);
            for (size_t i = 0; i < all_sessions.size(); i++) {
                if (all_sessions[i] != nullptr && ImGui::BeginTabItem(all_sessions[i]->name.c_str())) {
                    render_session_contents(all_sessions[i]);
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
        
        ImGui::Render();
        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(CLEAR_COLOR_R, CLEAR_COLOR_G, CLEAR_COLOR_B, CLEAR_COLOR_A);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }
    
    stopTileLoader();
    heat_stop_worker();
    
    if (tileWorker.joinable()) tileWorker.join();
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(gl);
    SDL_DestroyWindow(window);
    SDL_Quit();
}