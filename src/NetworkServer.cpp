#include "NetworkServer.h"
#include "TelemetryTypes.h"
#include "TelemetryParser.h"
#include "Database.h"
#include <zmq.hpp>
#include <iostream>
#include <fstream>
#include <mutex>
#include <vector>
#include <string>
#include <exception>

extern std::vector<TelemetrySession*> all_sessions;
extern std::mutex global_sessions_mtx;

void run_server() {
    zmq::context_t context(1);
    zmq::socket_t socket(context, zmq::socket_type::rep);
    socket.bind("tcp://*:5555");
    float first_time = -1;

    std::cout << "Server listening on port 5555..." << std::endl;
    
    while (true) {
        zmq::message_t request;
        if (!socket.recv(request, zmq::recv_flags::none)) continue;
        std::string msg(static_cast<char*>(request.data()), request.size());
        std::cout << "RECEIVED: " << msg << std::endl;
        
        try {
            std::ofstream logFile("log.json", std::ios::app);
            if (logFile.is_open()) { logFile << msg << std::endl; logFile.close(); }
            
            auto parts = split(msg, '|');
            float packet_time = parts.size() >= 1 ? parseTimeToSeconds(parts[0]) : 0;
            if (first_time < 0) first_time = packet_time;
            
            TelemetryPacket packet = parsePacket(msg, packet_time - first_time);
            saveToDatabase(packet);
            
            std::lock_guard<std::mutex> lock(global_sessions_mtx);
            if (!all_sessions.empty() && all_sessions[0] != nullptr) {
                all_sessions[0]->addPacket(packet);
            }
            
            socket.send(zmq::buffer("OK"), zmq::send_flags::none);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            socket.send(zmq::buffer("ERROR"), zmq::send_flags::none);
        }
    }
}