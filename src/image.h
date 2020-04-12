/** @file ppm.h
 *
 * File created by Peter Watkins (KE7IST) 1/8/18.
 * Derived from original project code.
 * Splat!
 * @copyright 1997 - 2018 John A. Magliacane (KD2BD) and contributors.
 * See revision control history for contributions.
 * This file is covered by the LICENSE.md file in the root of this project.
 */

#ifndef ppm_h
#define ppm_h

#include <string>
#include <vector>

class SplatRun;
class ElevationMap;
class Site;
class Region;

class Ppm {
private:
    const SplatRun &sr;
    const ElevationMap &em;
    std::string &filename;
    const std::vector<Site> &xmtr;

public:
    Ppm(const SplatRun &sr, std::string &filename, const std::vector<Site> &xmtr, const ElevationMap &em)
    : sr(sr),
    em(em),
    filename(filename),
    xmtr(xmtr)
    {}
    
void WritePPM();

void WritePPMLR(Region &region);

void WritePPMSS(Region &region);

void WritePPMDBM(Region &region);
    
private:
    
    void WriteKmlForPpm(const std::string &groundOverlayName, const std::string &description, bool writeScreenOverlay, const std::string &kmlfile, const std::string &mapfile, double north, double south, double east, double west, const std::string &ckfile);
    static void WriteGeo(const std::string &geofile, const std::string &mapfile, double north, double south, double east, double west, unsigned int width, unsigned int height);

};

#endif /* ppm_h */
