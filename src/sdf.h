/** @file sdf.h
 *
 * File created by Peter Watkins (KE7IST) 1/8/18.
 * Derived from original project code.
 * Splat!
 * @copyright 1997 - 2018 John A. Magliacane (KD2BD) and contributors.
 * See revision control history for contributions.
 * This file is covered by the LICENSE.md file in the root of this project.
 */

#ifndef sdf_h
#define sdf_h

#include <string>
#include <bzlib.h>

class ElevationMap;
class SplatRun;

class Sdf
{
private:
    std::string sdf_path;
    int	bzerror;
    const SplatRun &sr;
    
public:
    Sdf(const std::string &path, const SplatRun &sr)
    :sdf_path(path),
    sr(sr)
    {}
int LoadSDF_SDF(const std::string &name, ElevationMap &em);

int LoadSDF_BZ(const std::string &name, ElevationMap &em);

char LoadSDF(const std::string &name, ElevationMap &em);
    
    
private:
    char *BZfgets(BZFILE *bzfd, unsigned length);
};

#endif /* sdf_h */
