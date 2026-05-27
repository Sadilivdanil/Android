#pragma once

double LonToMercatorX(double lon);
double LatToMercatorY(double lat);
double MercatorXToLon(double mercatorX);
double MercatorYToLat(double mercatorY);
double MercatorXToTileX(double mercatorX, int zoom);
double MercatorYToTileY(double mercatorY, int zoom);
double TileXToMercatorX(int tileX, int zoom);
double TileYToMercatorY(int tileY, int zoom);
void computeVisibleArea(double centerLon, double centerLat, int zoom, 
                        double mapAspectRatio, double& minLon, double& maxLon,
                        double& minMercatorY, double& maxMercatorY);