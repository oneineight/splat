/** @file boundary_file.h
 *
 * File created by Peter Watkins (KE7IST) 1/9/18.
 * Derived from original project code.
 * Splat!
 * @copyright 1997 - 2018 John A. Magliacane (KD2BD) and contributors.
 * See revision control history for contributions.
 * This file is covered by the LICENSE.md file in the root of this project.
 */

#ifndef boundary_file_h
#define boundary_file_h

#include "elevation_map.h"
#include "path.h"

#include <string>

class BoundaryFile {
  private:
    const SplatRun &sr;

  public:
    BoundaryFile(const SplatRun &sr);

    void LoadBoundaries(const std::string &filename, ElevationMap &em);
};

#endif /* boundary_file_hpp */
