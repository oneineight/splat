/** @file report.h
 *
 * File created by Peter Watkins (KE7IST) 1/8/18.
 * Derived from original project code.
 * Splat!
 * @copyright 1997 - 2018 John A. Magliacane (KD2BD) and contributors.
 * See revision control history for contributions.
 * This file is covered by the LICENSE.md file in the root of this project.
 */

#ifndef report_h
#define report_h

#include <string>

class ElevationMap;
class SplatRun;
class Path;
class Site;
class PatFile;

class Report
{
private:
    const std::string dashes;
    
    const ElevationMap &em;
    const SplatRun &sr;
    Path &path;
    
public:
    Report(const ElevationMap &em, const SplatRun &sr, Path &path)
    : em(em), sr(sr), path(path),
    dashes("---------------------------------------------------------------------------")
    {}
    
    void PathReport(const Site &source, const Site &destination, const std::string &name, bool graph_it, elev_t elev[], const PatFile &pat, const Lrp &lrp);
    
    void SiteReport(const Site &xmtr);
    
private:
    void ObstructionAnalysis(const Site &xmtr, const Site &rcvr, double f, FILE *outfile);
};

#endif /* report_h */
