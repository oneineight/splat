/** @file elevation_map.h
 *
 * File created by Peter Watkins (KE7IST) 8/12/16.
 * Derived from original project code.
 * Splat!
 * @copyright 1997 - 2018 John A. Magliacane (KD2BD) and contributors.
 * See revision control history for contributions.
 * This file is covered by the LICENSE.md file in the root of this project.
 */

#ifndef elevation_map_h
#define elevation_map_h

#include <string>
#include <vector>
#include <stdio.h>
#include "itwom3.0.h"

class Path;
class SplatRun;
class Dem;
class Sdf;
class Site;
class Lrp;
class PatFile;

class ElevationMap
{
  
private:
    Path &path;
    
    const SplatRun &sr;
    double avgpathlen;
    int totalpaths;

    
public:
    std::vector<Dem> dem;
    int min_north;
    int max_north;
    int min_west;
    int max_west;
    int max_elevation;
    int min_elevation;
    
public:
    ElevationMap(Path &path, const SplatRun &sr);
    
    void LoadTopoData(int max_lon, int min_lon, int max_lat, int min_lat, Sdf &sdf);
    
    int PutMask(double lat, double lon, int value);
    
    int OrMask(double lat, double lon, int value);
    
    int GetMask(double lat, double lon) const;
    
    double haat(const Site &antenna) const;
    
    double GetElevation(const Site &location) const;
    
    int AddElevation(double lat, double lon, double height);
    
    double ElevationAngle(const Site &source, const Site &destination) const;
    
    double ElevationAngle2(const Site &source, const Site &destination, double er) const;
    
    double AverageTerrain(const Site &source, double azimuthx, double start_distance, double end_distance) const;
    
    void PlaceMarker(const Site &location);
    
    void PlotPath(const Site &source, const Site &destination, char mask_value);
    
    void PlotLOSMap(const Site &source, double altitude);
    
    void PlotLRMap(const Site &source, double altitude, const std::string &plo_filename, elev_t elev[], const PatFile &pat, const Lrp &lrp);
    
    int PutSignal(double lat, double lon, unsigned char signal);
    
    unsigned char GetSignal(double lat, double lon) const;
    
    ~ElevationMap();
    
private:
    void PlotLRPath(const Site &source, const Site &destination, unsigned char mask_value, FILE *fd, elev_t elev[], const PatFile &pat, const Lrp &lrp);

    bool FindMask(double lat, double lon, int &x, int &y, int &indx) const;
};

#endif /* elevation_map_h */
