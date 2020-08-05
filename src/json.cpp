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
    
void Json::WriteJSON(int argc, const char** argv) {
	int x;
	char report_name[80];
	FILE *fd = NULL;
	
	/* read in argv[] as an associative array.
	 * You can now access the list e.g. by args["o"] which will not fail if the argument is not set
	 * 
	 *	std::map<std::string, std::string>::iterator i;
	 *		for (i=args.begin(); i != args.end(); i++) {
	 *		std::cout << i->first << ": " << i->second << std::endl;
	 *	}
	 */
	typedef std::map<std::string,std::string> arg_t;
	arg_t args;
	
	std::string curr_arg = "";
	for(int i=0; i<argc; i++) {		// step through argv[] array
		std::string arg = argv[i];
		if(arg.find("-") == 0) {	// check if current argument has leading "-"
			curr_arg = arg.erase(0,1);	// remove leading "-" and save as new array entry
			args[curr_arg] = "";
		} else {
			if (curr_arg != "") {
				args[curr_arg] = arg;	// if no "-" was found the current argument is considered as a value to the previous argument
			}
		}
	}
	/* end argv[] reading */

    sprintf(report_name, "test.json");	// TODO: file name!

    for (x = 0; report_name[x] != 0; x++)
        if (report_name[x] == 32 || report_name[x] == 17 ||
            report_name[x] == 92 || report_name[x] == 42 ||
            report_name[x] == 47)
            report_name[x] = '_';

    fd = fopen(report_name, "w");

    fprintf(fd, "{\n");
    fprintf(fd, "\t\"splat\": \"%s\",\n", sr.splat_version.c_str());
    fprintf(fd, "\t\"name\": \"test\",\n");	// TODO: file name!
    fprintf(fd, "\t\"image\": {\n");
    fprintf(fd, "\t\t\"file\": \"test.png\",\n");	// TODO: image file name!
    fprintf(fd, "\t\t\"projection\": \"EPSG:3857\",\n");
    fprintf(fd, "\t\t\"bounds\": [[],[]],\n");	// TODO: bounds!
    fprintf(fd, "\t\t\"type\": \"dbm\",\n");
    fprintf(fd, "\t\t\"colormap\": {\n");
    fprintf(fd, "\t\t\t\"0\": \"#FF0000\",\n");
    fprintf(fd, "\t\t\t\"-10\": \"#FF8000\",\n");
    fprintf(fd, "\t\t},\n\t},\n");
    fprintf(fd, "\t\"qth\": {\n");
    fprintf(fd, "\t\t\"coordinates\": [],\n");
    fprintf(fd, "\t\t\"height\": 2\n");
    fprintf(fd, "\t},\n");
    fprintf(fd, "\t\"lrp\": {\n");
    fprintf(fd, "\t\t\"permittivity\": 15,\n");
    fprintf(fd, "\t},\n");
    fprintf(fd, "\t\"arguments\": {\n");
    
    std::map<std::string, std::string>::iterator i;
    for (i=args.begin(); i != args.end(); i++) {
		fprintf(fd, "\t\t\"%s\": \"%s\"\n", i->first.c_str(), i->second.c_str());
	}

    fprintf(fd, "\t}\n");
    fprintf(fd, "}");
    
	fclose(fd);

    fprintf(stdout, "\nJSON file written to: \"%s\"", report_name);

    fflush(stdout);
}
