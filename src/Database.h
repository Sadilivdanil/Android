#pragma once
#include "TelemetryTypes.h"

bool initDatabase();
void saveToDatabase(const TelemetryPacket& packet);
void closeDatabase();