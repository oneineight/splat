/** @file main.cpp
 *
 * SPLAT!: An RF Signal Path Loss And Terrain Analysis Tool
 * Project started in 1997 by John A. Magliacane (KD2BD)
 * File created by Peter Watkins (KE7IST) 6/17/15.
 * Derived from original project code.
 * Splat!
 * @copyright 1997 - 2018 John A. Magliacane (KD2BD) and contributors.
 * See revision control history for contributions.
 * This file is covered by the LICENSE.md file in the root of this project.
 */

#include "anf.h"
#include "antenna_pattern.h"
#include "boundary_file.h"
#include "city_file.h"
#include "dem.h"
#include "elevation_map.h"
#include "gnuplot.h"
#include "image.h"
#include "itwom3.0.h"
#include "kml.h"
#include "lrp.h"
#include "path.h"
#include "region.h"
#include "report.h"
#include "sdf.h"
#include "site.h"
#include "udt.h"
#include "utilities.h"
#include <bzlib.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

void check_allocation(void *ptr, string name, const SplatRun &sr);

int main(int argc, const char *argv[]) {
    size_t x, y, z = 0;
    int min_lat, min_lon, max_lat, max_lon, rxlat, rxlon, txlat, txlon,
        west_min, west_max, north_min, north_max;

//    string mapfile, elevation_file, height_file, longley_file, terrain_file,
//        udt_file, ani_filename, ano_filename, logfile, maxpages_str, proj;

//    vector<std::string> city_file;
//    vector<std::string> boundary_file;

//    vector<Site> tx_site;
//    Site rx_site;

    boost::optional<SplatRun> foo = SplatRun::parse_cli(argc, argv);
    if (!foo) {
      exit(0);
    }
    SplatRun sr = *foo;

    Sdf sdf(sr.sdf_path, sr);

    // Now print the header:
    cout << "\n\t\t--==[ Welcome To " << SplatRun::splat_name << " v"
         << SplatRun::splat_version << " ]==--\n\n";

    elev_t *elev = new elev_t[sr.arraysize + 10];
    check_allocation(elev, "elev", sr);

    ElevationMap *em_p = new ElevationMap(sr);
    check_allocation(em_p, "em_p", sr);

    Lrp lrp(sr.forced_erp, sr.forced_freq);
    BoundaryFile bf(sr);
    CityFile cf;
    Region region;

    if (!sr.ani_filename.empty()) {
	/* read alphanumeric output file from previous simulations if given */
	
        // TODO: Here's an instance where reading the LRParms may say to load
        // a PAT file but it's never used. Refactor this.
        AntennaPattern pat = AntennaPattern();

        // TODO: Why only the first TX site?
        bool loadPat;
        string patFilename;
        lrp.ReadLRParm(sr.tx_site[0], 0, loadPat, patFilename); /* Get ERP status */
        if (loadPat) {
            pat.LoadAntennaPattern(patFilename);
        }
        Anf anf(lrp, sr);

        y = anf.LoadANO(sr.ani_filename, sdf, *em_p);

        for (x = 0; x < sr.tx_site.size(); x++)
            em_p->PlaceMarker(sr.tx_site[x]);

        if (sr.rxsite)
            em_p->PlaceMarker(sr.rx_site);

        if (sr.boundary_file.size() > 0) {
            for (x = 0; x < sr.boundary_file.size(); x++) {
                bf.LoadBoundaries(sr.boundary_file[x], *em_p);
			}
            fprintf(stdout, "\n");
            fflush(stdout);
        }

        if (sr.city_file.size() > 0) {
            for (x = 0; x < sr.city_file.size(); x++) {
                cf.LoadCities(sr.city_file[x], *em_p);
			}
            fprintf(stdout, "\n");
            fflush(stdout);
        }

        Image image(sr, sr.mapfile, sr.tx_site, *em_p);
        if (lrp.erp == 0.0) {
            image.WriteCoverageMap(MAPTYPE_PATHLOSS, sr.imagetype, region);
        } else {
            if (sr.dbm)
                image.WriteCoverageMap(MAPTYPE_DBM, sr.imagetype, region);
            else
                image.WriteCoverageMap(MAPTYPE_DBUVM, sr.imagetype, region);
        }

        exit(0);
    }
    
    /* proceed for normal simulation */

    x = 0;
    y = 0;

    min_lat = 90;
    max_lat = -90;

    min_lon = (int)floor(sr.tx_site[0].lon);
    max_lon = (int)floor(sr.tx_site[0].lon);

    for (y = 0, z = 0; z < sr.tx_site.size(); z++) {
        txlat = (int)floor(sr.tx_site[z].lat);
        txlon = (int)floor(sr.tx_site[z].lon);

        if (txlat < min_lat)
            min_lat = txlat;

        if (txlat > max_lat)
            max_lat = txlat;

        if (Utilities::LonDiff(txlon, min_lon) < 0.0)
            min_lon = txlon;

        if (Utilities::LonDiff(txlon, max_lon) >= 0.0)
            max_lon = txlon;
    }

    if (sr.rxsite) {
        rxlat = (int)floor(sr.rx_site.lat);
        rxlon = (int)floor(sr.rx_site.lon);

        if (rxlat < min_lat)
            min_lat = rxlat;

        if (rxlat > max_lat)
            max_lat = rxlat;

        if (Utilities::LonDiff(rxlon, min_lon) < 0.0)
            min_lon = rxlon;

        if (Utilities::LonDiff(rxlon, max_lon) >= 0.0)
            max_lon = rxlon;
    }

    /* Load the required SDF files */
    em_p->LoadTopoData(max_lon, min_lon, max_lat, min_lat, sdf);

    if (sr.area_mode || sr.topomap) {
        for (z = 0; z < sr.tx_site.size(); z++) {
            /* "Ball park" estimates used to load any additional
             SDF files required to conduct this analysis. */

            sr.tx_range = sqrt(1.5 * (sr.tx_site[z].alt + em_p->GetElevation(sr.tx_site[z])));

            if (sr.LRmap) {
                sr.rx_range = sqrt(1.5 * sr.altitudeLR);
            } else {
                sr.rx_range = sqrt(1.5 * sr.altitude);
			}

            /* sr.deg_range determines the maximum
             amount of topo data we read */

            sr.deg_range = (sr.tx_range + sr.rx_range) / 57.0;

            /* sr.max_range regulates the size of the
             analysis.  A small, non-zero amount can
             be used to shrink the size of the analysis
             and limit the amount of topo data read by
             SPLAT!  A large number will increase the
             width of the analysis and the size of
             the map. */

            if (sr.max_range == 0.0) {
                sr.max_range = sr.tx_range + sr.rx_range;
			}
            sr.deg_range = sr.max_range / 57.0;

            /* Prevent the demand for a really wide coverage
             from allocating more "pages" than are available
             in memory. */

            switch (sr.maxpages) {
            case 1:
                sr.deg_limit = 0.125;
                break;

            case 2:
                sr.deg_limit = 0.25;
                break;

            case 4:
                sr.deg_limit = 0.5;
                break;

            case 9:
                sr.deg_limit = 1.0;
                break;

            case 16:
                sr.deg_limit = 1.5; /* WAS 2.0 */
                break;

            case 25:
                sr.deg_limit = 2.0; /* WAS 3.0 */
                break;

            case 36:
                sr.deg_limit = 2.5; /* New! */
                break;

            case 49:
                sr.deg_limit = 3.0; /* New! */
                break;

            case 64:
                sr.deg_limit = 3.5; /* New! */
                break;
            }

            if (fabs(sr.tx_site[z].lat) < 70.0) {
                sr.deg_range_lon = sr.deg_range / cos(DEG2RAD * sr.tx_site[z].lat);
            } else {
                sr.deg_range_lon = sr.deg_range / cos(DEG2RAD * 70.0);
			}
			
            /* Correct for squares in degrees not being square in miles */
            if (sr.deg_range > sr.deg_limit)
                sr.deg_range = sr.deg_limit;

            if (sr.deg_range_lon > sr.deg_limit)
                sr.deg_range_lon = sr.deg_limit;

            north_min = (int)floor(sr.tx_site[z].lat - sr.deg_range);
            north_max = (int)floor(sr.tx_site[z].lat + sr.deg_range);

            west_min = (int)floor(sr.tx_site[z].lon - sr.deg_range_lon);

            while (west_min < 0)
                west_min += 360;

            while (west_min >= 360)
                west_min -= 360;

            west_max = (int)floor(sr.tx_site[z].lon + sr.deg_range_lon);

            while (west_max < 0)
                west_max += 360;

            while (west_max >= 360)
                west_max -= 360;

            if (north_min < min_lat)
                min_lat = north_min;

            if (north_max > max_lat)
                max_lat = north_max;

            if (Utilities::LonDiff(west_min, min_lon) < 0.0)
                min_lon = west_min;

            if (Utilities::LonDiff(west_max, max_lon) >= 0.0)
                max_lon = west_max;
        }

        /* Load any additional SDF files, if required */
        em_p->LoadTopoData(max_lon, min_lon, max_lat, min_lat, sdf);
    }

    if (!sr.udt_file.empty()) {
        Udt udt(sr);
        udt.LoadUDT(sr.udt_file, *em_p);
    }

    /***** Let the SPLATting begin! *****/

    Report report(*em_p, sr);

    if (sr.pt2pt_mode) {
        em_p->PlaceMarker(sr.rx_site);

        string ext;
        if (sr.terrain_plot) {
            /* Extract extension (if present)
             from "terrain_file" */

            ext = Utilities::DivideExtension(sr.terrain_file, "png");
        }

        if (sr.elevation_plot) {
            /* Extract extension (if present)
             from "elevation_file" */
            ext = Utilities::DivideExtension(sr.elevation_file, "png");
        }

        if (sr.height_plot) {
            /* Extract extension (if present)
             from "height_file" */

            ext = Utilities::DivideExtension(sr.height_file, "png");
        }

        if (sr.longley_plot) {
            /* Extract extension (if present)
             from "longley_file" */

            ext = Utilities::DivideExtension(sr.longley_file, "png");
        }

        for (x = 0; x < sr.tx_site.size() && x < 4; x++) {
            em_p->PlaceMarker(sr.tx_site[x]);

            if (!sr.nolospath) {
                switch (x) {
                case 0:
                    em_p->PlotPath(sr.tx_site[x], sr.rx_site, 1);
                    break;

                case 1:
                    em_p->PlotPath(sr.tx_site[x], sr.rx_site, 8);
                    break;

                case 2:
                    em_p->PlotPath(sr.tx_site[x], sr.rx_site, 16);
                    break;

                case 3:
                    em_p->PlotPath(sr.tx_site[x], sr.rx_site, 32);
                }
            }

            if (!sr.nositereports)
                report.SiteReport(sr.tx_site[x]);

            if (sr.kml) {
                Kml kml(*em_p, sr);
                kml.WriteKML(sr.tx_site[x], sr.rx_site);
            }

            // If there's more than one TX site, put a dash-tx_site_number on
            // the end before the extension.
            std::string filename;
            std::ostringstream oss;
            if (sr.tx_site.size() > 1)
                oss << "-" << x + 1;
            oss << "." << ext;

            AntennaPattern pat;
            if (!sr.nositereports) {
                filename = sr.longley_file + oss.str();
                bool longly_file_exists = !sr.longley_file.empty();

                bool loadPat;
                string patFilename;
                lrp.ReadLRParm(sr.tx_site[x], longly_file_exists, loadPat,
                               patFilename);
                if (loadPat) {
                    pat.LoadAntennaPattern(patFilename);
                }
                report.PathReport(sr.tx_site[x], sr.rx_site, filename,
                                  longly_file_exists, elev, pat, lrp);
            } else {
                bool loadPat;
                string patFilename;
                lrp.ReadLRParm(sr.tx_site[x], 1, loadPat, patFilename);
                if (loadPat) {
                    pat.LoadAntennaPattern(patFilename);
                }
                report.PathReport(sr.tx_site[x], sr.rx_site, filename, true, elev,
                                  pat, lrp);
            }

            GnuPlot gnuPlot(sr);

            if (sr.terrain_plot) {
                filename = sr.terrain_file + oss.str();
                gnuPlot.GraphTerrain(sr.tx_site[x], sr.rx_site, filename, *em_p);
            }

            if (sr.elevation_plot) {
                filename = sr.elevation_file + oss.str();
                gnuPlot.GraphElevation(sr.tx_site[x], sr.rx_site, filename, *em_p);
            }

            if (sr.height_plot) {
                filename = sr.height_file + oss.str();
                gnuPlot.GraphHeight(sr.tx_site[x], sr.rx_site, filename,
                                    sr.fresnel_plot, sr.norm, *em_p, lrp);
            }
        }
    }

    if (sr.area_mode && !sr.topomap) {
        // Allocate the antenna pattern on the heap because it has a huge array
        // of floats that would otherwise be on the stack.
        AntennaPattern *p_pat = new AntennaPattern();
        for (x = 0; x < sr.tx_site.size(); x++) {

            if (sr.coverage) {
                em_p->PlotLOSMap(sr.tx_site[x], sr.altitude);
            } else {
                bool loadPat;
                string patFilename;
                char flag = lrp.ReadLRParm(sr.tx_site[x], 1, loadPat, patFilename);
                if (loadPat) {
                    p_pat->LoadAntennaPattern(patFilename);
                }

                if (flag) {
                    em_p->PlotLRMap(sr.tx_site[x], sr.altitudeLR, sr.ano_filename, *p_pat, lrp);
                }
            }

            report.SiteReport(sr.tx_site[x]);
        }
        delete p_pat;
    }

    if (sr.map || sr.topomap) {
        /* Label the map */

        if (!(sr.kml || sr.imagetype == IMAGETYPE_GEOTIFF)) {
            for (x = 0; x < sr.tx_site.size(); x++)
                em_p->PlaceMarker(sr.tx_site[x]);
        }

        if (sr.city_file.size() > 0) {
            CityFile cityFile;

            for (y = 0; y < sr.city_file.size(); y++)
                cityFile.LoadCities(sr.city_file[y], *em_p);

            fprintf(stdout, "\n");
            fflush(stdout);
        }

        /* Load city and county boundary data files */

        if (sr.boundary_file.size() > 0) {
            BoundaryFile boundaryFile(sr);

            for (y = 0; y < sr.boundary_file.size(); y++)
                boundaryFile.LoadBoundaries(sr.boundary_file[y], *em_p);

            fprintf(stdout, "\n");
            fflush(stdout);
        }

        /* Plot the map */
        Image image(sr, sr.mapfile, sr.tx_site, *em_p);
        if (sr.coverage || sr.pt2pt_mode || sr.topomap) {
            image.WriteCoverageMap(MAPTYPE_LOS, sr.imagetype, region);
            // TODO: PVW: Remove commented out line
            //image.WriteImage(sr.imagetype);
        } else {
            if (lrp.erp == 0.0)
                image.WriteCoverageMap(MAPTYPE_PATHLOSS, sr.imagetype, region);
            else if (sr.dbm)
                image.WriteCoverageMap(MAPTYPE_DBM, sr.imagetype, region);
            else
                image.WriteCoverageMap(MAPTYPE_DBUVM, sr.imagetype, region);
        }
    }    

    if (sr.command_line_log && !sr.logfile.empty()) {
        fstream fs;
        fs.open(sr.logfile.c_str(), fstream::out);

        // TODO: Should we fail silently if we can't open the logfile. Shouldn't
        // we WARN?
        if (fs) {
            for (x = 0; x < (size_t)argc; x++) {
                fs << argv[x] << " ";
			}
            fs << endl;
            fs.close();

            cout << "\nCommand-line parameter log written to: \"" << sr.logfile << "\"\n";
        }
    }

    cout << endl;
    
    delete em_p;

    // TODO: Why can't we clear. It complains about items already being
    // deleted?!
    // dem.clear();
    delete[] elev;

    return 0;
}

void check_allocation(void *ptr, string name, const SplatRun &sr) {
    if (ptr == NULL) {
        cerr << "\n\a*** ERROR: Could not allocate memory for " << name
             << " with -maxpages == " << sr.maxpages << "\n\n";
        exit(-1);
    }
}
