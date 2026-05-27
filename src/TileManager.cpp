#define STB_IMAGE_IMPLEMENTATION
#include "TileManager.h"
#include "Constants.h"
#include <curl/curl.h>
#include <stb_image.h>
#include <filesystem>
#include <queue>
#include <map>
#include <mutex>
#include <atomic>
#include <thread>
#include <sstream>
#include <fstream>
#include <iostream>

struct TileJob {
    std::string id;
    int zoom, x, y;
};

struct TextureData {
    GLuint id = 0;
    bool isLoading = false;
    std::vector<unsigned char> rgbaBlob;
    int width = 0, height = 0;
};

static std::map<std::string, TextureData> g_TileCache;
static std::queue<TileJob> g_JobQueue;
static std::mutex g_JobMutex;
static std::mutex g_CacheMutex;
static std::atomic<bool> g_Running{true};

static size_t onPullResponse(void *data, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    auto &blob = *static_cast<std::vector<unsigned char>*>(userp);
    auto *dataptr = static_cast<unsigned char*>(data);
    blob.insert(blob.end(), dataptr, dataptr + realsize);
    return realsize;
}

static bool receiveTile(int z, int x, int y, std::vector<unsigned char>& blob) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;
    
    std::ostringstream url;
    url << "https://tile.openstreetmap.org/" << z << '/' << x << '/' << y << ".png";
    
    curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "TelemetryServer/1.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &blob);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, onPullResponse);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    
    CURLcode res = curl_easy_perform(curl);
    bool ok = (res == CURLE_OK);
    curl_easy_cleanup(curl);
    return ok;
}

static bool loadTileFromFile(int z, int x, int y, std::vector<unsigned char>& blob) {
    std::string path = "tile_cache/" + std::to_string(z) + "/" + 
                       std::to_string(x) + "/" + std::to_string(y) + ".png";
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return false;
    
    blob.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return !blob.empty();
}

static void saveTileToFile(int z, int x, int y, const std::vector<unsigned char>& blob) {
    std::string dir = "tile_cache/" + std::to_string(z) + "/" + std::to_string(x);
    std::filesystem::create_directories(dir);
    std::string path = dir + "/" + std::to_string(y) + ".png";
    std::ofstream file(path, std::ios::binary);
    file.write(reinterpret_cast<const char*>(blob.data()), blob.size());
}

static std::vector<unsigned char> resizeTo256x256(const std::vector<unsigned char>& original, int w, int h) {
    const int TARGET = 256;
    std::vector<unsigned char> resized(TARGET * TARGET * 4, 0);
    double xr = (double)w / TARGET;
    double yr = (double)h / TARGET;
    
    for (int y = 0; y < TARGET; y++) {
        for (int x = 0; x < TARGET; x++) {
            int ox = (int)(x * xr);
            int oy = (int)(y * yr);
            int oi = (oy * w + ox) * 4;
            int ni = (y * TARGET + x) * 4;
            for (int c = 0; c < 4; c++) {
                resized[ni + c] = original[oi + c];
            }
        }
    }
    return resized;
}

void FetchWorker() {
    while (g_Running) {
        TileJob job;
        bool hasJob = false;
        {
            std::lock_guard<std::mutex> lock(g_JobMutex);
            if (!g_JobQueue.empty()) {
                job = g_JobQueue.front();
                g_JobQueue.pop();
                hasJob = true;
            }
        }
        
        if (hasJob) {
            std::vector<unsigned char> png;
            bool ok = false;
            
            if (loadTileFromFile(job.zoom, job.x, job.y, png)) {
                ok = true;
            } else if (receiveTile(job.zoom, job.x, job.y, png)) {
                saveTileToFile(job.zoom, job.x, job.y, png);
                ok = true;
            }
            
            if (ok) {
                int w, h, ch;
                unsigned char* data = stbi_load_from_memory(png.data(), png.size(), &w, &h, &ch, STBI_rgb_alpha);
                if (data) {
                    std::vector<unsigned char> rgba(data, data + w * h * 4);
                    auto resized = resizeTo256x256(rgba, w, h);
                    {
                        std::lock_guard<std::mutex> lock(g_CacheMutex);
                        auto& tex = g_TileCache[job.id];
                        tex.width = TILE_SIZE;
                        tex.height = TILE_SIZE;
                        tex.rgbaBlob = std::move(resized);
                        tex.isLoading = false;
                    }
                    stbi_image_free(data);
                }
            } else {
                std::lock_guard<std::mutex> lock(g_CacheMutex);
                g_TileCache[job.id].isLoading = false;
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}

void requestTile(const std::string& id, int zoom, int x, int y) {
    std::lock_guard<std::mutex> lock(g_CacheMutex);
    if (g_TileCache.find(id) != g_TileCache.end()) return;
    
    TextureData td;
    td.isLoading = true;
    g_TileCache[id] = td;
    
    TileJob job{id, zoom, x, y};
    std::lock_guard<std::mutex> lockJob(g_JobMutex);
    g_JobQueue.push(job);
}

bool getTileTexture(const std::string& id, GLuint& outId) {
    std::lock_guard<std::mutex> lock(g_CacheMutex);
    auto it = g_TileCache.find(id);
    if (it == g_TileCache.end()) return false;
    
    if (!it->second.rgbaBlob.empty()) {
        glGenTextures(1, &it->second.id);
        glBindTexture(GL_TEXTURE_2D, it->second.id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, it->second.width, it->second.height,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, it->second.rgbaBlob.data());
        it->second.rgbaBlob.clear();
    }
    
    if (it->second.id != 0) {
        outId = it->second.id;
        return true;
    }
    return false;
}

void stopTileLoader() { 
    g_Running = false; 
}