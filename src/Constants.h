#pragma once
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DB_HOST "localhost"
#define DB_PORT "5432"
#define DB_NAME "telemetry_db"
#define DB_USER "postgres"
#define DB_PASSWORD "1234"

constexpr int TILE_SIZE = 256;
constexpr int MAX_ZOOM = 18;
constexpr int MIN_ZOOM = 1;
constexpr int DEFAULT_ZOOM = 10;

constexpr double NOVOSIBIRSK_LAT = 55.0302;
constexpr double NOVOSIBIRSK_LON = 82.9204;

constexpr float CLEAR_COLOR_R = 0.1f;
constexpr float CLEAR_COLOR_G = 0.1f;
constexpr float CLEAR_COLOR_B = 0.1f;
constexpr float CLEAR_COLOR_A = 1.0f;