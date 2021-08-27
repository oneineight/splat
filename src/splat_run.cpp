/** @file splat_run.cpp
 *
 * File created by Peter Watkins (KE7IST) 6/17/15.
 * Derived from original project code.
 * Splat!
 * @copyright 1997 - 2018 John A. Magliacane (KD2BD) and contributors.
 * See revision control history for contributions.
 * This file is covered by the LICENSE.md file in the root of this project.
 */

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <fstream>

#include <boost/optional.hpp>

#include "splat_run.h"
#include "itwom3.0.h"

using namespace std;

const std::string SplatRun::splat_name = "SPLAT!";
const std::string SplatRun::splat_version = "2.0-alpha";

SplatRun::SplatRun() {
      maxpages = 16;
      arraysize = -1;

      propagation_model = PROP_ITM;
      hd_mode = false;
      coverage = false;
      LRmap = false;
      terrain_plot = false;
      elevation_plot = false;
      height_plot = false;
      map = false;
      longley_plot = false;
      norm = false;
      topomap = false;
      geo = false;
      kml = false;
      pt2pt_mode = false;
      area_mode = false;
      ngs = false;
      nolospath = false;
      nositereports = false;
      fresnel_plot = true;
      command_line_log = false;
      rxsite = false;
      metric = true;
      dbm = false;
      bottom_legend = true;
      smooth_contours = false;

      altitude = 0.0;
      altitudeLR = 0.0;
      tx_range = 0.0;
      rx_range = 0.0;
      deg_range = 0.0;
      deg_limit = 0.0;
      max_range = 0.0;
      clutter = 0.0;
      forced_erp = -1.0;
      forced_freq = 0.0;
      fzone_clearance = 0.6;
      contour_threshold = 0;

//      rx_site.lat = 91.0;
//      rx_site.lon = 361.0;

      earthradius = EARTHRADIUS;

#ifdef HAVE_LIBPNG
      imagetype = IMAGETYPE_PNG;
#else
      imagetype = IMAGETYPE_PPM;
#endif

      projection = PROJ_EPSG_4326;
      multithread = true;
      verbose = 1;    
      sdf_delimiter = "_";

//    string mapfile, elevation_file, height_file, longley_file, terrain_file,
//        udt_file, ani_filename, ano_filename, logfile, maxpages_str, proj;
//
//    vector<std::string> city_file;
//    vector<std::string> boundary_file;
//
//    vector<Site> tx_site;
//    Site rx_site;
}

boost::optional<SplatRun> SplatRun::parse_cli(int argc, const char *argv[]) {
    size_t x, y, z = 0;

    char *env = NULL;

//    string mapfile, elevation_file, height_file, longley_file, terrain_file,
//        udt_file, ani_filename, ano_filename, logfile, maxpages_str, proj;

//    vector<std::string> city_file;
//    vector<std::string> boundary_file;

//    vector<Site> tx_site;
//    Site rx_site;

    SplatRun sr;

    if (argc == 1) {
        cout
            << "\n\t\t --==[ " << SplatRun::splat_name << " v"
            << SplatRun::splat_version
            << " Available Options... ]==--\n\n"
               "       -t txsite(s).qth (max of 4 with -c, max of 30 with -L)\n"
               "       -r rxsite.qth\n"
               "       -c plot LOS coverage of TX(s) with an RX antenna at X "
               "feet/meters AGL\n"
               "       -L plot path loss map of TX based on an RX at X "
               "feet/meters AGL\n"
               "       -s filename(s) of city/site file(s) to import (5 max)\n"
               "       -b filename(s) of cartographic boundary file(s) to "
               "import (5 max)\n"
               "       -p filename of terrain profile graph to plot\n"
               "       -e filename of terrain elevation graph to plot\n"
               "       -h filename of terrain height graph to plot\n"
               "       -H filename of normalized terrain height graph to plot\n"
               "       -l filename of path loss graph to plot\n"
               "       -o filename of topographic map to generate (without "
               "suffix)\n"
               "       -u filename of user-defined terrain file to import\n"
               "       -d sdf file directory path (overrides path in "
               "~/.splat_path file)\n"
               "       -m earth radius multiplier\n"
               "       -n do not plot LOS paths in maps\n"
               "       -N do not produce unnecessary site or obstruction "
               "reports\n"
               "       -f frequency for Fresnel zone calculation (MHz)\n"
               "       -R modify default range for -c or -L "
               "(miles/kilometers)\n"
               "       -v N verbosity level. Default is 1. Set to 0 to quiet "
               "everything.\n"
               "      -st use a single CPU thread (classic mode)\n"
               "      -hd Use High Definition mode. Requires 1-deg SDF files.\n"
               "      -sc display smooth rather than quantized contour levels\n"
               "      -db threshold beyond which contours will not be "
               "displayed\n"
               "      -nf do not plot Fresnel zones in height plots\n"
               "      -fz Fresnel zone clearance percentage (default = 60)\n"
               "      -gc ground clutter height (feet/meters)\n"
               "     -jpg when generating maps, create jpgs instead of pngs or "
               "ppms\n"
#ifdef HAVE_LIBPNG
               "     -ppm when generating maps, create ppms instead of pngs or "
               "jpgs\n"
#endif
               "     -tif create geotiff instead of png or jpeg\n"
               "     -ngs display greyscale topography as white in images\n"
               "     -erp override ERP in .lrp file (Watts)\n"
               "     -ano name of alphanumeric output file\n"
               "     -ani name of alphanumeric input file\n"
               "     -udt name of user defined terrain input file\n"
               "     -kml generate Google Earth (.kml) compatible output\n"
               "     -geo generate an Xastir .geo georeference file (with "
               "image output)\n"
               "     -dbm plot signal power level contours rather than field "
               "strength\n"
               "     -log copy command line string to this output file\n"
               "   -gpsav preserve gnuplot temporary working files after "
               "SPLAT! execution\n"
               "   -itwom invoke the ITWOM model instead of using "
               "Longley-Rice\n"
               "  -imperial employ imperial rather than metric units for all "
               "user I/O\n"
               "-maxpages ["
            << sr.maxpages
            << "] Maximum Analysis Region capability: 1, 4, 9, 16, 25, 36, 49, "
               "64 \n"
               "  -sdelim ["
            << sr.sdf_delimiter
            << "] Lat and lon delimeter in SDF filenames \n"
               "\n"
               "See the documentation for more details.\n\n";

        return boost::none;
    }

    /* Scan for command line arguments */
    y = argc - 1;

    for (x = 1; x <= y; x++) {
        if (strcmp(argv[x], "-R") == 0) {
            z = x + 1;

            if (z <= y && argv[z][0] && argv[z][0] != '-') {
                sscanf(argv[z], "%lf", &sr.max_range);

                if (sr.max_range < 0.0)
                    sr.max_range = 0.0;

                if (sr.max_range > 1000.0)
                    sr.max_range = 1000.0;
            }
        }

        if (strcmp(argv[x], "-m") == 0) {
            z = x + 1;

            if (z <= y && argv[z][0] && argv[z][0] != '-') {
                sscanf(argv[z], "%lf", &sr.er_mult);

                if (sr.er_mult < 0.1)
                    sr.er_mult = 1.0;

                if (sr.er_mult > 1.0e6)
                    sr.er_mult = 1.0e6;

                sr.earthradius *= sr.er_mult;
            }
        }

        if (strcmp(argv[x], "-v") == 0) {
            z = x + 1;

            if (z < (size_t)argc && argv[z][0] && argv[z][0] != '-') {
                int verbose;
                sscanf(argv[z], "%d", &verbose);
                sr.verbose = verbose != 0;
            }
        }

        if (strcmp(argv[x], "-gc") == 0) {
            z = x + 1;

            if (z <= y && argv[z][0] && argv[z][0] != '-') {
                sscanf(argv[z], "%lf", &sr.clutter);

                if (sr.clutter < 0.0)
                    sr.clutter = 0.0;
            }
        }

        if (strcmp(argv[x], "-fz") == 0) {
            z = x + 1;

            if (z <= y && argv[z][0] && argv[z][0] != '-') {
                sscanf(argv[z], "%lf", &sr.fzone_clearance);

                if (sr.fzone_clearance < 0.0 || sr.fzone_clearance > 100.0)
                    sr.fzone_clearance = 60.0;

                sr.fzone_clearance /= 100.0;
            }
        }

        if (strcmp(argv[x], "-o") == 0) {
            z = x + 1;

            if (z <= y && argv[z][0] && argv[z][0] != '-')
                sr.mapfile = argv[z];
            sr.map = true;
        }

        if (strcmp(argv[x], "-log") == 0) {
            z = x + 1;

            if (z <= y && argv[z][0] && argv[z][0] != '-')
                sr.logfile = argv[z];

            sr.command_line_log = true;
        }

        if (strcmp(argv[x], "-udt") == 0) {
            z = x + 1;

            if (z <= y && argv[z][0] && argv[z][0] != '-')
                sr.udt_file = argv[z];
        }

        if (strcmp(argv[x], "-c") == 0) {
            z = x + 1;

            if (z <= y && argv[z][0] && argv[z][0] != '-') {
                sscanf(argv[z], "%lf", &sr.altitude);
                sr.map = true;
                sr.coverage = true;
                sr.area_mode = true;
            }
        }

        if (strcmp(argv[x], "-db") == 0 || strcmp(argv[x], "-dB") == 0) {
            z = x + 1;

            if (z <= y && argv[z][0]) /* A minus argument is legal here */
                sscanf(argv[z], "%d", &sr.contour_threshold);
        }

        if (strcmp(argv[x], "-p") == 0) {
            z = x + 1;

            if (z <= y && argv[z][0] && argv[z][0] != '-') {
                sr.terrain_file = argv[z];
                sr.terrain_plot = true;
                sr.pt2pt_mode = true;
            }
        }

        if (strcmp(argv[x], "-e") == 0) {
            z = x + 1;

            if (z <= y && argv[z][0] && argv[z][0] != '-') {
                sr.elevation_file = argv[z];
                sr.elevation_plot = true;
                sr.pt2pt_mode = true;
            }
        }

        if (strcmp(argv[x], "-h") == 0 || strcmp(argv[x], "-H") == 0) {
            z = x + 1;

            if (z <= y && argv[z][0] && argv[z][0] != '-') {
                sr.height_file = argv[z];
                sr.height_plot = true;
                sr.pt2pt_mode = true;
            }

            sr.norm = strcmp(argv[x], "-H") == 0 ? true : false;
        }

		bool imagetype_set = false;
#ifdef HAVE_LIBPNG
        if (strcmp(argv[x], "-ppm") == 0) {
            if (imagetype_set && sr.imagetype != IMAGETYPE_PPM) {
                fprintf(stdout, "-jpg and -ppm are exclusive options, ignoring -ppm.\n");
            } else {
                sr.imagetype = IMAGETYPE_PPM;
                imagetype_set = true;
            }
        }
#endif
#ifdef HAVE_LIBGDAL
        if (strcmp(argv[x], "-tif") == 0) {
            if (imagetype_set && sr.imagetype != IMAGETYPE_PPM) {
                fprintf(stdout, "-tif and -ppm are exclusive options, ignoring -ppm.\n");
            } else {
                sr.imagetype = IMAGETYPE_GEOTIFF;
                imagetype_set = true;
            }
        }
#endif
#ifdef HAVE_LIBJPEG
        if (strcmp(argv[x], "-jpg") == 0) {
            if (imagetype_set && sr.imagetype != IMAGETYPE_JPG) {
#ifdef HAVE_LIBPNG
                fprintf(stdout, "-jpg and -ppm are exclusive options, ignoring -jpg.\n");
#else
                fprintf(stdout, "-jpg and -png are exclusive options, ignoring -jpg.\n");
#endif
            } else {
                sr.imagetype = IMAGETYPE_JPG;
                imagetype_set = true;
            }
        }
#endif

#ifdef HAVE_LIBGDAL
        if (strcmp(argv[x], "-proj") == 0) {
            if (sr.imagetype == IMAGETYPE_GEOTIFF || sr.imagetype == IMAGETYPE_PNG || sr.imagetype == IMAGETYPE_JPG) {
				z = x + 1;
				if (z <= y && argv[z][0] && argv[z][0] != '-') {
					if(strcmp(argv[z], "epsg:3857") == 0) {
						sr.projection = PROJ_EPSG_3857;
					} else if(strcmp(argv[z], "epsg:4326") == 0) {
						sr.projection = PROJ_EPSG_4326;
					} else {
						cerr << "Ignoring unknown projection " << argv[z] << " and taking epsg:4326 instead.\n";
					}
				}	
            } else {
                cerr << "-proj supports only gdal output formats. Please use -png, -tif or -jpg.\n";
            }
        }
#endif

        if (strcmp(argv[x], "-imperial") == 0)
            sr.metric = false;

        if (strcmp(argv[x], "-gpsav") == 0)
            sr.gpsav = true;

        if (strcmp(argv[x], "-geo") == 0)
            sr.geo = true;

        if (strcmp(argv[x], "-kml") == 0)
            sr.kml = true;
            
        if (strcmp(argv[x], "-nf") == 0)
            sr.fresnel_plot = false;

        if (strcmp(argv[x], "-ngs") == 0)
            sr.ngs = true;

        if (strcmp(argv[x], "-n") == 0)
            sr.nolospath = true;

        if (strcmp(argv[x], "-dbm") == 0)
            sr.dbm = true;

        if (strcmp(argv[x], "-sc") == 0)
            sr.smooth_contours = true;

        if (strcmp(argv[x], "-st") == 0)
            sr.multithread = false;

        if (strcmp(argv[x], "-itwom") == 0)
            sr.propagation_model = PROP_ITWOM;

        if (strcmp(argv[x], "-N") == 0) {
            sr.nolospath = true;
            sr.nositereports = true;
        }

        if (strcmp(argv[x], "-d") == 0) {
            z = x + 1;

            if (z <= y && argv[z][0] && argv[z][0] != '-')
                sr.sdf_path = argv[z];
        }

        if (strcmp(argv[x], "-t") == 0) {
            /* Read Transmitter Location */

            z = x + 1;

            while (z <= y && argv[z][0] && argv[z][0] != '-' && sr.tx_site.size() < 30) {
                string txfile = argv[z];
                sr.tx_site.push_back(Site(txfile));
                z++;
            }

            z--;
        }

        if (strcmp(argv[x], "-L") == 0) {
            z = x + 1;

            if (z <= y && argv[z][0] && argv[z][0] != '-') {
                sscanf(argv[z], "%lf", &sr.altitudeLR);
                sr.map = true;
                sr.LRmap = true;
                sr.area_mode = true;

                if (sr.coverage)
                    fprintf(stdout,"c and L are exclusive options, ignoring L.\n");
            }
        }

        if (strcmp(argv[x], "-l") == 0) {
            z = x + 1;

            if (z <= y && argv[z][0] && argv[z][0] != '-') {
                sr.longley_file = argv[z];
                sr.longley_plot = true;
                sr.pt2pt_mode = true;
            }
        }

        if (strcmp(argv[x], "-r") == 0) {
            /* Read Receiver Location */

            z = x + 1;

            if (z <= y && argv[z][0] && argv[z][0] != '-') {
                string rxfile = argv[z];
                sr.rx_site.LoadQTH(rxfile);
                sr.rxsite = true;
                sr.pt2pt_mode = true;
            }
        }

        if (strcmp(argv[x], "-s") == 0) {
            /* Read city file(s) */

            z = x + 1;

            while (z <= y && argv[z][0] && argv[z][0] != '-') {
                sr.city_file.push_back(argv[z]);
                z++;
            }

            z--;
        }

        if (strcmp(argv[x], "-b") == 0) {
            /* Read Boundary File(s) */

            z = x + 1;

            while (z <= y && argv[z][0] && argv[z][0] != '-') {
                sr.boundary_file.push_back(argv[z]);
                z++;
            }

            z--;
        }

        if (strcmp(argv[x], "-f") == 0) {
            z = x + 1;

            if (z <= y && argv[z][0] && argv[z][0] != '-') {
                sscanf(argv[z], "%lf", &(sr.forced_freq));

                if (sr.forced_freq < 20.0)
                    sr.forced_freq = 0.0;

                if (sr.forced_freq > 20.0e3)
                    sr.forced_freq = 20.0e3;
            }
        }

        if (strcmp(argv[x], "-erp") == 0) {
            z = x + 1;

            if (z <= y && argv[z][0] && argv[z][0] != '-') {
                sscanf(argv[z], "%lf", &(sr.forced_erp));

                if (sr.forced_erp < 0.0)
                    sr.forced_erp = -1.0;
            }
        }

        if (strcmp(argv[x], "-ano") == 0) {
            z = x + 1;

            if (z <= y && argv[z][0] && argv[z][0] != '-')
                sr.ano_filename = argv[z];
        }

        if (strcmp(argv[x], "-ani") == 0) {
            z = x + 1;

            if (z <= y && argv[z][0] && argv[z][0] != '-')
                sr.ani_filename = argv[z];
        }

        if (strcmp(argv[x], "-maxpages") == 0) {
            z = x + 1;

            if (z <= y && argv[z][0] && argv[z][0] != '-') {
                std::string maxpages_str = argv[z];
                if (sscanf(maxpages_str.c_str(), "%d", &sr.maxpages) != 1) {
                    cerr << "\n"
                         << 7 << "*** ERROR: Could not parse maxpages: "
                         << maxpages_str << "\n\n";
                    exit(-1);
                }
            }
        }

        if (strcmp(argv[x], "-sdelim") == 0) {
            z = x + 1;

            if (z <= y && argv[z][0] && argv[z][0] != '-') {
                sr.sdf_delimiter = argv[z];
            }
        }

        if (strcmp(argv[x], "-hd") == 0) {
            sr.hd_mode = true;
        }
    } /* end of command line argument scanning */


    /* Perform some error checking on the arguments
     and switches parsed from the command-line.
     If an error is encountered, print a message
     and exit gracefully. */

    if (sr.tx_site.size() == 0) {
        fprintf(stderr, "\n%c*** ERROR: No transmitter site(s) specified!\n\n", 7);
        exit(-1);
    }

    for (x = 0, y = 0; x < sr.tx_site.size(); x++) {
        if (sr.tx_site[x].lat == 91.0 && sr.tx_site[x].lon == 361.0) {
            fprintf(stderr, "\n*** ERROR: Transmitter site #%lu not found!", x + 1);
            y++;
        }
    }

    if (y) {
        fprintf(stderr, "%c\n\n", 7);
        exit(-1);
    }

    if (!sr.coverage && !sr.LRmap && sr.ani_filename.empty() &&
        sr.rx_site.lat == 91.0 && sr.rx_site.lon == 361.0) {
        if (sr.max_range != 0.0 && sr.tx_site.size() != 0) {
            /* Plot topographic map of radius "sr.max_range" */
            sr.map = false;
            sr.topomap = true;
        } else {
            fprintf(stderr, "\n%c*** ERROR: No receiver site found or specified!\n\n", 7);
            exit(-1);
        }
    }
    
    /* check if the output map should have a bottom legend */
    // TODO: PVW: LOS maps don't use a legend. Does sr.coverage detect those correctly?
    if (sr.kml || sr.geo || (sr.imagetype == IMAGETYPE_GEOTIFF) || sr.coverage) {
		sr.bottom_legend = false;
	} else {
		sr.bottom_legend = true;
	}

    switch (sr.maxpages) {
    case 1:
        if (!sr.hd_mode) {
            fprintf(
                stderr,
                "\n%c*** ERROR: -maxpages must be >= 4 if not in HD mode!\n\n",
                7);
            exit(-1);
        }
        sr.arraysize = 5092;
        break;
    case 4:
        sr.arraysize = sr.hd_mode ? 14844 : 4950;
        break;

    case 9:
        sr.arraysize = sr.hd_mode ? 32600 : 10870;
        break;

    case 16:
        sr.arraysize = sr.hd_mode ? 57713 : 19240;
        break;

    case 25:
        sr.arraysize = sr.hd_mode ? 90072 : 30025;
        break;

    case 36:
        sr.arraysize = sr.hd_mode ? 129650 : 43217;
        break;

    case 49:
        sr.arraysize = sr.hd_mode ? 176437 : 58813;
        break;

    case 64:
        sr.arraysize = sr.hd_mode ? 230430 : 76810;
        break;
    default:
        fprintf(stderr,
                "\n%c*** ERROR: -maxpages must be one of 1, 4, 9, 16, 25, 36, "
                "49, 64\n\n",
                7);
        exit(-1);
    }

    sr.ippd = sr.hd_mode ? 3600 : 1200; /* pixels per degree (integer) */

    int degrees = (int)sqrt((int)sr.maxpages);

    cout << "This invocation of " << SplatRun::splat_name
         << " supports analysis over a region of " << degrees << " square \n" << ((degrees == 1) ? "degree" : "degrees")
		 << " of terrain, and computes signal levels using ITWOM Version " << ITWOMVersion() << ".\n\n";

    sr.ppd = (double)sr.ippd; /* pixels per degree (double)  */
    sr.dpp = 1.0 / sr.ppd;    /* degrees per pixel */
    sr.mpi = sr.ippd - 1;     /* maximum pixel index per degree */

    /* No major errors were detected.  Whew!  :-) */

    /* Adjust input parameters if -imperial option is not used */

    if (sr.metric) {
        sr.altitudeLR /= METERS_PER_FOOT; /* meters --> feet */
        sr.max_range /= KM_PER_MILE;      /* kilometers --> miles */
        sr.altitude /= METERS_PER_FOOT;   /* meters --> feet */
        sr.clutter /= METERS_PER_FOOT;    /* meters --> feet */
    }

    /* If no SDF path was specified on the command line (-d), check
     for a path specified in the $HOME/.splat_path file.  If the
     file is not found, then sr.sdf_path[] remains NULL, and the
     current working directory is assumed to contain the SDF
     files. */

    if (sr.sdf_path.empty()) {
        env = getenv("HOME");
        std::string config_path = env;
        config_path += "/.splat_path";
        fstream fs;
        fs.open(config_path.c_str(), fstream::in);

        if (fs) {
            getline(fs, sr.sdf_path);
            fs.close();
        }
    }

    /* Ensure a trailing '/' is present in sr.sdf_path */

    if (!sr.sdf_path.empty() && (*sr.sdf_path.rbegin() != '/')) {
        sr.sdf_path += '/';
    }

    return sr;
}
