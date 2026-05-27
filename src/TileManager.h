#pragma once
#include <string>
#include <GL/glew.h>

void FetchWorker();
void stopTileLoader();
void requestTile(const std::string& id, int zoom, int x, int y);
bool getTileTexture(const std::string& id, GLuint& outId);