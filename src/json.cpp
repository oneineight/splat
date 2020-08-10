/** @file jsonl.cpp
 *
 * File created by Stefan Erhardt (DL1NFS) 3/8/2020.
 * Derived from original project code.
 * Splat!
 * @copyright 1997 - 2018 John A. Magliacane (KD2BD) and contributors.
 * See revision control history for contributions.
 * This file is covered by the LICENSE.md file in the root of this project.
 */

#include "json.h"
#include "elevation_map.h"
#include "path.h"
#include "site.h"
#include "splat_run.h"
#include "utilities.h"
#include <iostream>
#include <bits/stdc++.h>

Json::Json(const ElevationMap &em, const SplatRun &sr)
    : path(sr.arraysize, sr.ppd), em(em), sr(sr) {}
    
void Json::WriteJSON(arg_t args, Site tx_site, Lrp lrp, std::string mapfile) {
	int x;
	char report_name[80];
	FILE *fd = NULL;

    sprintf(report_name, "%s.json", mapfile.c_str());

    for (x = 0; report_name[x] != 0; x++)
        if (report_name[x] == 32 || report_name[x] == 17 ||
            report_name[x] == 92 || report_name[x] == 42 ||
            report_name[x] == 47)
            report_name[x] = '_';

    fd = fopen(report_name, "w");

    fprintf(fd, "{\n");
    fprintf(fd, "\t\"splat\": \"%s\",\n", sr.splat_version.c_str());
    fprintf(fd, "\t\"name\": \"%s\",\n", tx_site.name.c_str());
    fprintf(fd, "\t\"image\": {\n");
    fprintf(fd, "\t\t\"file\": \"%s.png\",\n", mapfile.c_str());
    fprintf(fd, "\t\t\"projection\": \"EPSG:3857\",\n");
    fprintf(fd, "\t\t\"bounds\": [[%d, %d],[%d, %d]],\n", em.min_north, 360-em.max_west, em.max_north, 360-em.min_west);
    fprintf(fd, "\t\t\"unit\": \"dbm\",\n");	//TODO: determine unit (dBm or dBuV/m)
    fprintf(fd, "\t\t\"colormap\": {\n");
    fprintf(fd, "\t\t\t\"0\": \"#FF0000\",\n");
    fprintf(fd, "\t\t\t\"-10\": \"#FF8000\",\n");	//TODO: determine colormap
    fprintf(fd, "\t\t}\n\t},\n");
    fprintf(fd, "\t\"qth\": {\n");
    fprintf(fd, "\t\t\"coordinates\": [%f, %f],\n", tx_site.lat, 360-tx_site.lon);
    fprintf(fd, "\t\t\"height\": %.2f\n", tx_site.alt);
    fprintf(fd, "\t},\n");
    fprintf(fd, "\t\"lrp\": {\n");
    fprintf(fd, "\t\t\"permittivity\": %.1f,\n", lrp.eps_dielect);
    fprintf(fd, "\t\t\"conductivity\": %.3f,\n", lrp.sgm_conductivity);
    fprintf(fd, "\t\t\"bending\": %.1f,\n", lrp.eno_ns_surfref);
    fprintf(fd, "\t\t\"frequency\": %.1f,\n", lrp.frq_mhz);
    fprintf(fd, "\t\t\"climate\": %d,\n", lrp.radio_climate);
    fprintf(fd, "\t\t\"polarization\": %d,\n", lrp.pol);
    fprintf(fd, "\t\t\"location_variability\": %.2f,\n", lrp.conf);
    fprintf(fd, "\t\t\"time_variability\": %.2f,\n", lrp.rel);
    fprintf(fd, "\t\t\"erp\": %.1f,\n", lrp.erp);
    fprintf(fd, "\t},\n");
    fprintf(fd, "\t\"arguments\": {\n");
    
    std::map<std::string, std::string>::iterator i;
    for (i=args.begin(); i != args.end(); i++) {
		fprintf(fd, "\t\t\"%s\": \"%s\",\n", i->first.c_str(), i->second.c_str());
	}

    fprintf(fd, "\t}\n");
    fprintf(fd, "}");
    
	fclose(fd);

    fprintf(stdout, "\nJSON file written to: \"%s\"", report_name);

    fflush(stdout);
}
