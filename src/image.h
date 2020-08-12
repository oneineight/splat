/** @file image.h
 *
 * File created by Peter Watkins (KE7IST) 1/8/18.
 * Derived from original project code.
 * Splat!
 * @copyright 1997 - 2018 John A. Magliacane (KD2BD) and contributors.
 * See revision control history for contributions.
 * This file is covered by the LICENSE.md file in the root of this project.
 */

#ifndef image_h
#define image_h

#include "splat_run.h"
#include "elevation_map.h"
#include "site.h"
#include "region.h"
#include "imagewriter.h"

#include <string>
#include <vector>

class Image {
  private:
    const SplatRun &sr;
    const ElevationMap &em;
    std::string &filename;
    const std::vector<Site> &xmtr;

  public:
    Image(const SplatRun &sr, std::string &filename,
          const std::vector<Site> &xmtr, const ElevationMap &em)
        : sr(sr), em(em), filename(filename), xmtr(xmtr) {}

    void WriteImage(ImageType imagetype);

    void WriteImage_PathLoss(ImageType imagetype, Region &region);

    void WriteImage_dBuVm(ImageType imagetype, Region &region);

    void WriteImage_dBm(ImageType imagetype, Region &region);

  private:
    void WriteKmlForImage(const std::string &groundOverlayName,
                          const std::string &description,
                          bool writeScreenOverlay, const std::string &kmlfile,
                          const std::string &mapfile, double north,
                          double south, double east, double west,
                          const std::string &ckfile);
    static void WriteGeo(const std::string &geofile, const std::string &mapfile,
                         double north, double south, double east, double west,
                         unsigned int width, unsigned int height);
};

#endif /* image_h */
