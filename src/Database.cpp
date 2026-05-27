#include "Database.h"
#include "TelemetryTypes.h"
#include "Constants.h"
#include <libpq-fe.h>
#include <iostream>

static PGconn* db_connection = nullptr;

bool initDatabase() {
    std::string conn_info = "host=" DB_HOST " port=" DB_PORT 
                          " dbname=" DB_NAME " user=" DB_USER 
                          " password=" DB_PASSWORD;
    db_connection = PQconnectdb(conn_info.c_str());
    if (PQstatus(db_connection) != CONNECTION_OK) {
        std::cerr << "ОШИБКА подключения: " << PQerrorMessage(db_connection) << std::endl;
        return false;
    }
    std::cout << "Подключение к БД УСПЕШНО!" << std::endl;
    return true;
}

void saveToDatabase(const TelemetryPacket& packet) {
    if (!db_connection) return;
    
    for (const auto& cell : packet.cells) {
        std::string query = "INSERT INTO cell_telemetry (timestamp, latitude, longitude, altitude, accuracy, cell_type, pci, rsrp, rsrq, rssi, sinr) VALUES ('" +
            packet.timestamp + "', " +
            std::to_string(packet.lat) + ", " +
            std::to_string(packet.lon) + ", " +
            std::to_string(packet.alt) + ", " +
            std::to_string(packet.accuracy) + ", '" +
            cell.type + "', " +
            std::to_string(cell.pci) + ", " +
            std::to_string(cell.rsrp) + ", " +
            std::to_string(cell.rsrq) + ", " +
            std::to_string(cell.rssi) + ", " +
            std::to_string(cell.sinr) + ")";
        
        PGresult* res = PQexec(db_connection, query.c_str());
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            std::cerr << "ОШИБКА вставки: " << PQresultErrorMessage(res) << std::endl;
        }
        PQclear(res);
    }
}

void closeDatabase() {
    if (db_connection) PQfinish(db_connection);
}