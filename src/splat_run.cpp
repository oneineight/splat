/** @file splat_run.cpp
 *
 * File created by Peter Watkins (KE7IST) 6/17/15.
 * Derived from original project code.
 * Splat!
 * @copyright 1997 - 2018 John A. Magliacane (KD2BD) and contributors.
 * See revision control history for contributions.
 * This file is covered by the LICENSE.md file in the root of this project.
 */

#include "splat_run.h"
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

using namespace std;

const std::string SplatRun::splat_name = "SPLAT!";
const std::string SplatRun::splat_version = "2.0-alpha";

//class Foo {
//public: 
//  Foo(char x, int y) {}
//  Foo(int y) : Foo('a', y) {}
//};

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
      rx_site.lat = 91.0;
      rx_site.lon = 361.0;
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
}
