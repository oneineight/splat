# SPLAT! Utilities

Utilities for use with SPLAT! software are found under the
`utils` directory.  They include the following:


## srtm2sdf
The `srtm2sdf` utility generates SPLAT Data Files (SDFs) from STS-99
Space Shuttle Topography Mission (SRTM) elevation data files.  This
data is of a much higher quality than that contained in older USGS
Digital Elevation Models of the same resolution.  However, many SRTM
Version 2 elevation models contain data "spikes", "voids", and "wells"
that are the consequence of the radar mapping process.

The `srtm2sdf` utility has the ability to detect and replace SRTM data
outliers with equivalent usgs2sdf derived SDF data (see `usgs2sdf` below).
If such data is not available, SRTM outliers are handled either through
adjacent pixel averaging, or by threshold limiting using user-specified
limits.  Of all three methods, the USGS-derived SDF replacement method
yields the best results.

The `srtm2sdf` utility processes SRTM-3 3-arc second resolution data
or use with SPLAT! operating in standard definition mode.

SRTM-3 Version 2 Elevation Data files may be downloaded from:

	http://dds.cr.usgs.gov/srtm/version2_1/SRTM3/

Files available at this site are ZIP compressed, and must be
uncompressed (using `unzip`, or `gunzip -S .zip`) prior to being
processed by `srtm2sdf`.

The `srtm2sdf` utility accepts command-line options as follows:

-d:  used to specify the directory path to the location of `usgs2sdf`
     derived SDF files that are to be used to replace outliers found
     in the SRTM data file.  The `-d` option overrides the default path
     specified in your `$HOME/.splat_path file`.

-n:  used to specify the elevation (in meters) below which SRTM data
     is either replaced with `usgs2sdf`-derived SDF data, or averaged
     among adjacent elevation data points.  The default threshold for
     the replacement limit is sea-level (0 meters).  Unless elevations
     below sea-level are known to exist for the region being
     processed by the `srtm2sdf` utility, the `-n` option need not be
     specified.

Some examples of srtm2sdf use:

    srtm2sdf N40W074.hgt

    srtm2sdf -d /cdrom/sdf N40W074.hgt

    srtm2sdf -d /dev/null N40W074.hgt (/dev/null prevents USGS data
		replacement from taking place)

    srtm2sdf -n -5 N40W074.hgt

In all cases, SDF files are written into the current working directory.

The srtm2sdf utility may also be used to convert 3-arc second SRTM data
in Band Interleaved by Line (.BIL) format for use with SPLAT!  This data 
is available via the web at: http://seamless.usgs.gov/website/seamless/

Once the region of the world has been selected at this site, select the
"Define Download Area By Coordinates" button under "Downloads".  Proceed
to request the download of the region(s) desired in 1 degree by 1 degree
regions only, and make sure bounding coordinates entered fall exactly on
whole numbers of latitude and longitude (no decimal fractions of a degree).

Select the "Add Area" button at the bottom.

On the next screen, select "Modify Data Request".  On the subsequent screen,
de-select the National Elevation Dataset (NED) format and select "SRTM 3 arc
sec - Shuttle Radar Topography Mission [Finished]".  Change the format from
ArcGrid to BIL, and from HTML to TXT.

Select the "Save Changes and Return To Summary" button.

Select the "Download" button, and save the file once it has been sent to
your web browser.

Uncompressing the file will generate a directory containing all files
contained within the downloaded archive.  Move into the directory and
invoke srtm2sdf as described above with the filename having the .bil
extension given as its argument.  Finally, move or copy the generated
.sdf file to your SPLAT! working directory.


## srtm2sdf-hd
The `srtm2sdf-hd` utility operates in an identical manner as `srtm2sdf`,
but is used to generate HD SDF files from SRTM-1 one-arc second
resolution data files for use with SPLAT! HD.  SRTM-1 data files
are available for the United States and its territories and
possessions, and may be downloaded from:

	http://dds.cr.usgs.gov/srtm/version2_1/SRTM1/


## usgs2sdf
The `usgs2sdf` utility takes as an argument the name of an uncompressed
and record delimited Digital Elevation Model Data (DEM) downloaded from
the US Geological Survey, and generates a SPLAT Data File (SDF) compatible
with SPLAT! Software.  `usgs2sdf` may be invoked manually, or via the
postdownload script.


## postdownload
`postdownload` is a front-end to the usgs2sdf utility.  `postdownload`
takes as an argument the name of the gzipped Digital Elevation Model
(DEM) downloaded from the US Geological Survey (ie: wilmington-w.gz).
`postdownload` uncompresses the DEM file, adds necessary record delimiters,
and invokes `usgs2sdf` to produce a SPLAT! Data File (SDF).

USGS Digital Elevation Models may be downloaded from:

    http://edcftp.cr.usgs.gov/pub/data/DEM/250/

Invoke `postdownload` with the name of each DEM file downloaded to
produce a database of SPLAT Data Files.


## citydecoder
This utility reads certain U.S. Census Bureau files to produce city/site
data files that can be imported into SPLAT! software to annotate
SPLAT!-generated maps.  Incorporated Places/Census Designated Places
data files for use with citydecoder may be downloaded from:

http://web.archive.org/web/20130201115142/http://www.census.gov/geo/www/cob/bdy_files.html

(Formerly: http://www.census.gov/geo/www/cob/bdy_files.html)

Similarly, County Subdivision files containing the names, locations,
and boundaries of cities, and smaller towns, townships, and boroughs
may be downloaded from:

http://web.archive.org/web/20130331172800/http://www.census.gov/geo/www/cob/cs2000.html

(Formerly: http://www.census.gov/geo/www/cob/cs2000.html)

and processed with the `citydecoder` utility. 

Please select the ARC/INFO Ungenerate (ASCII) Metadata Cartographic Boundary
Files from these sites and unzip them prior to processing them with
`citydecoder`:

	unzip -a pl34_d00_ascii.zip
	unzip -a cs34_d00_ascii.zip

U.S. Census files are cataloged by the two digit FIPS code for the region
(state) they represent.  A list of FIPS codes is included in fips.txt
under splat-1.4.1/utils for your convenience.

`citydecoder` takes as an argument the two-letter file prefix plus the FIPS
code of the region or state being processed.  For example:

	citydecoder pl34

reads files "pl34_d00.dat" and "pl34_d00a.dat" that were extracted after
the unzipping process, and generates a list of city names and geographical
coordinates for the state of New Jersey (FIPS code 34).  This data may be
sorted and written to a file (cities.nj.dat) in the following manner:

	citydecoder pl34 | sort > cities.nj.dat

In a similar manner, unzipped County Subdivision files may be processed
with the `citydecoder` utility to produce a file containing locations and
names of towns, townships, and boroughs:

	citydecoder cs34 | sort > townships.nj.dat

`citydecoder` can also process more than one file or file type per invocation,
and produce a merged output file as follows:

	citydecoder pl34 cs34 | sort > everything.in.nj.dat

Be advised that any redundancy that may exist between "cs" and "pl" files
will be reflected in the merged output file, so some manual editing of
the output file may be necessary.

 
 ## fontdata
The `fontdata` utility reads Slackware gzipped console font data
to create the `fontdata.h` file required for compilation of SPLAT!.
Font data of the type needed by this utility may be found under
`/usr/lib/kbd/consolefonts` (Slackware < 8), or under
`/usr/share/kbd/consolefonts` (Slackware >= 8.0).

A default `fontdata.h` file is already included in with SPLAT!, and is
a derivative of the s.fnt console font type available under Slackware.
fontdata takes as an argument the name of the file containing the
gzipped compressed console fonts:

	fontdata s.fnt.gz


## bearing
The bearing utility reads a pair of .qth files specified on the command
line, and returns the azimuth bearing and great circle path distance between
the two points specified.  A `-metric` switch is available so that distances
can be provided in kilometers rather than statute miles.  SPLAT! provides
similar distance and bearing information between two specific site locations.
The bearing utility, however, provides the information quickly and easily
over great distances without having to run SPLAT!
