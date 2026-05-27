#include <zmq.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>

int main() {
    std::cout << "=== TEST SERVER ===" << std::endl;
    
    try {
        zmq::context_t context(1);
        zmq::socket_t socket(context, zmq::socket_type::rep);
        socket.bind("tcp://*:5555");
        
        std::cout << "Server started on port 5555" << std::endl;
        
        while (true) {
            zmq::message_t request;
            socket.recv(request, zmq::recv_flags::none);
            
            std::string received(static_cast<char*>(request.data()), request.size());
            std::cout << "RECEIVED: " << received << std::endl;
            
            std::ofstream file("test_log.txt", std::ios::app);
            file << received << std::endl;
            file.close();
            
            std::cout << "Saved to test_log.txt" << std::endl;
            
            socket.send(zmq::str_buffer("OK"), zmq::send_flags::none);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}