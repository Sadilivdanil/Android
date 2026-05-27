#pragma once
#include "TelemetryTypes.h"
#include <string>
#include <vector>
#include <mutex>

void load_history_tab(const std::string& filename, 
                      std::vector<TelemetrySession*>& sessions, 
                      std::mutex& mtx);
void render_session_contents(TelemetrySession* s);