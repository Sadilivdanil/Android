#include "Database.h"
#include "NetworkServer.h"
#include "GuiManager.h"
#include <thread>

std::vector<TelemetrySession*> all_sessions;
std::mutex global_sessions_mtx;

int main() {
    initDatabase();
    
    TelemetrySession* live_session = new TelemetrySession();
    live_session->name = "LIVE STREAM";
    all_sessions.push_back(live_session);
    
    std::thread server_thread(run_server);
    run_gui();
    
    if (server_thread.joinable()) server_thread.join();
    
    for (auto* session : all_sessions) delete session;
    closeDatabase();
    
    return 0;
}