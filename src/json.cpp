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
#include <fstream>
#include <bits/stdc++.h>

Json::Json(const ElevationMap &em, const SplatRun &sr)
    : path(sr.arraysize, sr.ppd), em(em), sr(sr) {}
    
void Json::WriteJSON(arg_t args, Site tx_site, Lrp lrp, std::string mapfile) {
	int x;
	char report_name[80];

    sprintf(report_name, "%s.json", mapfile.c_str());

    for (x = 0; report_name[x] != 0; x++)
        if (report_name[x] == 32 || report_name[x] == 17 ||
            report_name[x] == 92 || report_name[x] == 42 ||
            report_name[x] == 47)
            report_name[x] = '_';
    
    std::ofstream reportfile(report_name);
    
    reportfile << "{\n";
    reportfile << "\t\"splat\": \"" << sr.splat_version.c_str() << "\",\n";
    reportfile << "\t\"name\": \"" << tx_site.name.c_str() << "\",\n";
    reportfile << "\t\"image\": {\n";
    reportfile << "\t\t\"file\": \"" << mapfile.c_str() << ".png\",\n";
    reportfile << "\t\t\"projection\": \"EPSG:3857\",\n";
    reportfile << "\t\t\"bounds\": [[" << em.min_north << ", " << 360-em.max_west << "],[" << em.max_north << ", " << 360-em.min_west << "]],\n";
    reportfile << "\t\t\"unit\": \"dbm\",\n";	//TODO: determine unit (dBm or dBuV/m)
    reportfile << "\t\t\"colormap\": {\n";
    reportfile << "\t\t\t\"0\": \"#FF0000\",\n";
    reportfile << "\t\t\t\"-10\": \"#FF8000\",\n";	//TODO: determine colormap
    reportfile << "\t\t}\n\t},\n";
    reportfile << "\t\"qth\": {\n";
    reportfile << "\t\t\"coordinates\": [" << tx_site.lat << ", " << 360-tx_site.lon << "],\n";
    reportfile << "\t\t\"height\": " << tx_site.alt << "\n";
    reportfile << "\t},\n";
    reportfile << "\t\"lrp\": {\n";
    reportfile << "\t\t\"permittivity\": " << lrp.eps_dielect << ",\n";
    reportfile << "\t\t\"conductivity\": " << lrp.sgm_conductivity << ",\n";
    reportfile << "\t\t\"bending\": " << lrp.eno_ns_surfref << ",\n";
    reportfile << "\t\t\"frequency\": " << lrp.frq_mhz << ",\n";
    reportfile << "\t\t\"climate\": " << lrp.radio_climate << ",\n";
    reportfile << "\t\t\"polarization\": " << lrp.pol << ",\n";
    reportfile << "\t\t\"location_variability\": " << lrp.conf << ",\n";
    reportfile << "\t\t\"time_variability\": " << lrp.rel << ",\n";
    reportfile << "\t\t\"erp\": " << lrp.erp << ",\n";
    reportfile << "\t},\n";
    reportfile << "\t\"arguments\": {\n";
    
    std::map<std::string, std::string>::iterator i;
    for (i=args.begin(); i != args.end(); i++) {
		reportfile << "\t\t\"" <<  i->first.c_str() << "\": \"" << i->second.c_str() << "\",\n";
	}

    reportfile << "\t},\n";
    reportfile << "},\n";

	reportfile.close();
	
	std::cout << "\nJSON file written to: " << report_name;

    fflush(stdout);
}
