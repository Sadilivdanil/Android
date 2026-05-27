#pragma once
#include <string>
#include <vector>
#include <map>
#include <mutex>

struct CellData {
    std::string type;
    int pci = 0;
    int rsrp = -140;
    int rsrq = -20;
    int ssRsrp = -140;
    int dbm = -120;
    int rssi = -120;
    int sinr = 0;
};

struct TelemetryPacket {
    std::string timestamp;
    double lat = 0, lon = 0;
    double alt = 0, accuracy = 0;
    std::vector<CellData> cells;
    float elapsed_time = 0;
};

struct TelemetrySession {
    std::string name;
    TelemetryPacket current_packet;
    std::mutex mtx;
    std::vector<float> time_history;
    std::vector<float> lat_values, lon_values;  
    std::map<int, std::vector<float>> pci_signal_history;
    std::map<int, std::string> pci_types;
    
    void addPacket(const TelemetryPacket& packet);
    void clear();
};