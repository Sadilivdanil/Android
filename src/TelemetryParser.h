#pragma once
#include "TelemetryTypes.h"
#include <string>
#include <vector>

std::vector<std::string> split(const std::string& s, char d);
int extractInt(const std::string& str, const std::string& param);
float parseTimeToSeconds(const std::string& timeStr);
TelemetryPacket parsePacket(const std::string& msg, float elapsed);