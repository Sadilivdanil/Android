#include "Mercator.h"
#include "Constants.h"
#include <cmath>

double LonToMercatorX(double lon) {
    return lon;
}

double LatToMercatorY(double lat) {
    double lat_rad = lat * M_PI / 180.0;
    return std::log(std::tan(lat_rad) + 1.0 / std::cos(lat_rad)) * 180.0 / M_PI;
}

double MercatorXToLon(double mercatorX) {
    return mercatorX;
}

double MercatorYToLat(double mercatorY) {
    return std::atan(std::sinh(mercatorY * M_PI / 180.0)) * 180.0 / M_PI;
}

double MercatorXToTileX(double mercatorX, int zoom) {
    return (0.5 + mercatorX / 360.0) * (1 << zoom);
}

double MercatorYToTileY(double mercatorY, int zoom) {
    return (0.5 - mercatorY / 360.0) * (1 << zoom);
}

double TileXToMercatorX(int tileX, int zoom) {
    return (tileX / static_cast<double>(1 << zoom) - 0.5) * 360.0;
}

double TileYToMercatorY(int tileY, int zoom) {
    return (0.5 - tileY / static_cast<double>(1 << zoom)) * 360.0;
}

void computeVisibleArea(double centerLon, double centerLat, int zoom, 
                        double mapAspectRatio, double& minLon, double& maxLon,
                        double& minMercatorY, double& maxMercatorY) {
    double centerMercY = LatToMercatorY(centerLat);
    double viewWidth = 360.0 / (1 << (zoom - 1));
    double viewHeight = viewWidth / mapAspectRatio;
    
    minLon = centerLon - viewWidth / 2;
    maxLon = centerLon + viewWidth / 2;
    minMercatorY = centerMercY - viewHeight / 2;
    maxMercatorY = centerMercY + viewHeight / 2;
}