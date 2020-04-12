/** @file image.cpp
 *
 * File created by Peter Watkins (KE7IST) 1/8/18.
 * Derived from original project code.
 * Splat!
 * @copyright 1997 - 2018 John A. Magliacane (KD2BD) and contributors.
 * See revision control history for contributions.
 * This file is covered by the LICENSE.md file in the root of this project.
 */

#include <string>
#include <vector>
#include <fstream>
#include <cmath>
#include <cstdio>
#include "utilities.h"
#include "site.h"
#include "path.h"
#include "dem.h"
#include "pat_file.h"
#include "splat_run.h"
#include "lrp.h"
#include "region.h"
#include "sdf.h"
#include "elevation_map.h"
#include "image.h"
#include "fontdata.h"

using namespace std;

void Image::WriteImage()
{
    /* This function generates a topographic map in Portable Pix Map
     (PPM) format based on logarithmically scaled topology data,
     as well as the content of flags held in the mask[][] array.
     The image created is rotated counter-clockwise 90 degrees
     from its representation in em.dem[][] so that north points
     up and east points right in the image generated. */
    
    string basename, mapfile, geofile, kmlfile;
    bool found;
    unsigned char mask;
    unsigned width, height, terrain;
    int indx, x, y, x0=0, y0=0;
    double lat, lon, conversion, one_over_gamma,
    north, south, east, west, minwest;
    FILE *fd;
    
    one_over_gamma=1.0/GAMMA;
    conversion=255.0/pow((double)(em.max_elevation-em.min_elevation),one_over_gamma);
    
    width=(unsigned)(sr.ippd*Utilities::ReduceAngle(em.max_west-em.min_west));
    height=(unsigned)(sr.ippd*Utilities::ReduceAngle(em.max_north-em.min_north));
    
    if (filename.empty())
    {
        basename = Utilities::Basename(xmtr[0].filename);
        filename = basename + ".ppm";
    }
    else
    {
        basename = Utilities::Basename(filename);
    }
    
    mapfile = basename + ".ppm";
    geofile = basename + ".geo";
    kmlfile = basename + ".kml";
    
    minwest=((double)em.min_west)+sr.dpp;
    
    if (minwest>360.0)
        minwest-=360.0;
    
    north=(double)em.max_north-sr.dpp;
    south=(double)em.min_north;
    east=(minwest<180.0?-minwest:360.0-em.min_west);
    west=(double)(em.max_west<180?-em.max_west:360-em.max_west);
    
    if (!sr.kml&& sr.geo)
    {
        WriteGeo(geofile, mapfile, north, south, east, west, width, height);
    }
    
    if (sr.kml&& !sr.geo)
    {
        WriteKmlForImage(SplatRun::splat_name + " Line-of-Sight Contour", "Line-of-Sight Contour", false, kmlfile, mapfile, north, south, east, west, "");
    }
    
    fd=fopen(mapfile.c_str(),"wb");
    
    fprintf(fd,"P6\n%u %u\n255\n",width,height);
    fprintf(stdout,"\nWriting \"%s\" (%ux%u pixmap image)... ",mapfile.c_str(),width,height);
    fflush(stdout);
    
    for (y = 0, lat=north; y<(int)height; y++, lat=north-(sr.dpp*(double)y))
    {
        for (x=0, lon=em.max_west; x<(int)width; x++, lon=(double)em.max_west-(sr.dpp*(double)x))
        {
            if (lon<0.0)
                lon+=360.0;
            
            for (indx=0, found = false; indx<sr.maxpages && !found;)
            {
                x0=(int)rint(sr.ppd*(lat-(double)em.dem[indx].min_north));
                y0=sr.mpi-(int)rint(sr.ppd*(Utilities::LonDiff((double)em.dem[indx].max_west,lon)));
                
                if (x0>=0 && x0<=sr.mpi && y0>=0 && y0<=sr.mpi)
                    found = true;
                else
                    indx++;
            }
            
            if (found)
            {
                mask=em.dem[indx].mask[x0 *sr.ippd + y0];
                
                if (mask&2)
                /* Text Labels: Red */
                    fprintf(fd,"%c%c%c",255,0,0);
                
                else if (mask&4)
                /* County Boundaries: Light Cyan */
                    fprintf(fd,"%c%c%c",128,128,255);
                
                else switch (mask&57)
                {
                    case 1:
                        /* TX1: Green */
                        fprintf(fd,"%c%c%c",0,255,0);
                        break;
                        
                    case 8:
                        /* TX2: Cyan */
                        fprintf(fd,"%c%c%c",0,255,255);
                        break;
                        
                    case 9:
                        /* TX1 + TX2: Yellow */
                        fprintf(fd,"%c%c%c",255,255,0);
                        break;
                        
                    case 16:
                        /* TX3: Medium Violet */
                        fprintf(fd,"%c%c%c",147,112,219);
                        break;
                        
                    case 17:
                        /* TX1 + TX3: Pink */
                        fprintf(fd,"%c%c%c",255,192,203);
                        break;
                        
                    case 24:
                        /* TX2 + TX3: Orange */
                        fprintf(fd,"%c%c%c",255,165,0);
                        break;
                        
                    case 25:
                        /* TX1 + TX2 + TX3: Dark Green */
                        fprintf(fd,"%c%c%c",0,100,0);
                        break;
                        
                    case 32:
                        /* TX4: Sienna 1 */
                        fprintf(fd,"%c%c%c",255,130,71);
                        break;
                        
                    case 33:
                        /* TX1 + TX4: Green Yellow */
                        fprintf(fd,"%c%c%c",173,255,47);
                        break;
                        
                    case 40:
                        /* TX2 + TX4: Dark Sea Green 1 */
                        fprintf(fd,"%c%c%c",193,255,193);
                        break;
                        
                    case 41:
                        /* TX1 + TX2 + TX4: Blanched Almond */
                        fprintf(fd,"%c%c%c",255,235,205);
                        break;
                        
                    case 48:
                        /* TX3 + TX4: Dark Turquoise */
                        fprintf(fd,"%c%c%c",0,206,209);
                        break;
                        
                    case 49:
                        /* TX1 + TX3 + TX4: Medium Spring Green */
                        fprintf(fd,"%c%c%c",0,250,154);
                        break;
                        
                    case 56:
                        /* TX2 + TX3 + TX4: Tan */
                        fprintf(fd,"%c%c%c",210,180,140);
                        break;
                        
                    case 57:
                        /* TX1 + TX2 + TX3 + TX4: Gold2 */
                        fprintf(fd,"%c%c%c",238,201,0);
                        break;
                        
                    default:
                        if (sr.ngs)  /* No terrain */
                            fprintf(fd,"%c%c%c",255,255,255);
                        else
                        {
                            /* Sea-level: Medium Blue */
                            if (em.dem[indx].data[x0 *sr.ippd + y0]==0)
                                fprintf(fd,"%c%c%c",0,0,170);
                            else
                            {
                                /* Elevation: Greyscale */
                                terrain=(unsigned)(0.5+pow((double)(em.dem[indx].data[x0 *sr.ippd + y0]-em.min_elevation),one_over_gamma)*conversion);
                                fprintf(fd,"%c%c%c",terrain,terrain,terrain);
                            }
                        }
                }
            }
            
            else
            {
                /* We should never get here, but if */
                /* we do, display the region as black */
                
                fprintf(fd,"%c%c%c",0,0,0);
            }
        }
    }
    
    fclose(fd);
    fprintf(stdout,"Done!\n");
    fflush(stdout);
}


void Image::WriteImageLR(Region &region)
{
    /* This function generates a topographic map in Portable Pix Map
     (PPM) format based on the content of flags held in the mask[][]
     array (only).  The image created is rotated counter-clockwise
     90 degrees from its representation in em.dem[][] so that north
     points up and east points right in the image generated. */
    
    string basename, mapfile, geofile, kmlfile, ckfile;
    bool found;
    unsigned width, height, red, green, blue, terrain=0;
    unsigned char mask, cityorcounty;
    int indx, x, y, z, colorwidth, x0 = 0, y0 = 0, loss, level,
    hundreds, tens, units, match;
    double lat, lon, conversion, one_over_gamma,
    north, south, east, west, minwest;
    FILE *fd;
    
    one_over_gamma=1.0/GAMMA;
    conversion=255.0/pow((double)(em.max_elevation-em.min_elevation),one_over_gamma);
    
    width=(unsigned)(sr.ippd*Utilities::ReduceAngle(em.max_west-em.min_west));
    height=(unsigned)(sr.ippd*Utilities::ReduceAngle(em.max_north-em.min_north));
    
    region.LoadLossColors(xmtr[0]);
    
    if (filename.empty())
    {
        basename = Utilities::Basename(xmtr[0].filename);
        filename = basename + ".ppm";
    }
    else
    {
        basename = Utilities::Basename(filename);
    }
    
    mapfile = basename + ".ppm";
    geofile = basename + ".geo";
    kmlfile = basename + ".kml";
    ckfile  = basename + "-ck.ppm";
    
    minwest=((double)em.min_west)+sr.dpp;
    
    if (minwest>360.0)
        minwest-=360.0;
    
    north=(double)em.max_north-sr.dpp;
    
    if (sr.kml|| sr.geo)
        south=(double)em.min_north;	/* No bottom legend */
    else
        south=(double)em.min_north-(30.0/sr.ppd); /* 30 pixels for bottom legend */
    
    east=(minwest<180.0?-minwest:360.0-em.min_west);
    west=(double)(em.max_west<180?-em.max_west:360-em.max_west);
    
    if (sr.kml==0 && sr.geo)
    {
        WriteGeo(geofile, mapfile, north, south, east, west, width, height);
    }
    
    if (sr.kml&& sr.geo==0)
    {
        WriteKmlForImage("SPLAT! Path Loss Overlay", xmtr[0].name + " Transmitter Path Loss Overlay", true, kmlfile, mapfile, north, south, east, west, ckfile);
    }
    
    fd=fopen(mapfile.c_str(),"wb");
    
    if (sr.kml|| sr.geo)
    {
        /* No bottom legend */
        
        fprintf(fd,"P6\n%u %u\n255\n",width,height);
        fprintf(stdout,"\nWriting \"%s\" (%ux%u pixmap image)... ",mapfile.c_str(),width,height);
    }
    
    else
    {
        /* Allow space for bottom legend */
        
        fprintf(fd,"P6\n%u %u\n255\n",width,height+30);
        fprintf(stdout,"\nWriting \"%s\" (%ux%u pixmap image)... ",mapfile.c_str(),width,height+30);
    }
    
    fflush(stdout);
    
    for (y = 0, lat=north; y<(int)height; y++, lat=north-(sr.dpp*(double)y))
    {
        for (x=0, lon=em.max_west; x<(int)width; x++, lon=em.max_west-(sr.dpp*(double)x))
        {
            if (lon<0.0)
                lon+=360.0;
            
            for (indx=0, found = false; indx<sr.maxpages && !found;)
            {
                x0=(int)rint(sr.ppd*(lat-(double)em.dem[indx].min_north));
                y0=sr.mpi-(int)rint(sr.ppd*(Utilities::LonDiff((double)em.dem[indx].max_west,lon)));
                
                if (x0>=0 && x0<=sr.mpi && y0>=0 && y0<=sr.mpi)
                    found = true;
                else
                    indx++;
            }
            
            if (found)
            {
                mask=em.dem[indx].mask[x0 *sr.ippd + y0];
                loss=(em.dem[indx].signal[x0 *sr.ippd + y0]);
                cityorcounty=0;
                
                match=255;
                
                red=0;
                green=0;
                blue=0;
                
                if (loss<=region.level[0])
                    match=0;
                else
                {
                    for (z=1; (z<region.levels && match==255); z++)
                    {
                        if (loss>=region.level[z-1] && loss<region.level[z])
                            match=z;
                    }
                }
                
                if (match<region.levels)
                {
                    if (sr.smooth_contours && match>0)
                    {
                        red=(unsigned)Utilities::interpolate(region.color[match-1][0],region.color[match][0],region.level[match-1],region.level[match],loss);
                        green=(unsigned)Utilities::interpolate(region.color[match-1][1],region.color[match][1],region.level[match-1],region.level[match],loss);
                        blue=(unsigned)Utilities::interpolate(region.color[match-1][2],region.color[match][2],region.level[match-1],region.level[match],loss);
                    }
                    
                    else
                    {
                        red=region.color[match][0];
                        green=region.color[match][1];
                        blue=region.color[match][2];
                    }
                }
                
                if (mask&2)
                {
                    /* Text Labels: Red or otherwise */
                    
                    if (red>=180 && green<=75 && blue<=75 && loss!=0)
                        fprintf(fd,"%c%c%c",255^red,255^green,255^blue);
                    else
                        fprintf(fd,"%c%c%c",255,0,0);
                    
                    cityorcounty=1;
                }
                
                else if (mask&4)
                {
                    /* County Boundaries: Black */
                    
                    fprintf(fd,"%c%c%c",0,0,0);
                    
                    cityorcounty=1;
                }
                
                if (cityorcounty==0)
                {
                    if (loss==0 || (sr.contour_threshold!=0 && loss>abs(sr.contour_threshold)))
                    {
                        if (sr.ngs)  /* No terrain */
                            fprintf(fd,"%c%c%c",255,255,255);
                        else
                        {
                            /* Display land or sea elevation */
                            
                            if (em.dem[indx].data[x0 *sr.ippd + y0]==0)
                                fprintf(fd,"%c%c%c",0,0,170);
                            else
                            {
                                terrain=(unsigned)(0.5+pow((double)(em.dem[indx].data[x0 *sr.ippd + y0]-em.min_elevation),one_over_gamma)*conversion);
                                fprintf(fd,"%c%c%c",terrain,terrain,terrain);
                            }
                        }
                    }
                    
                    else
                    {
                        /* Plot path loss in color */
                        
                        if (red!=0 || green!=0 || blue!=0)
                            fprintf(fd,"%c%c%c",red,green,blue);
                        
                        else  /* terrain / sea-level */
                        {
                            if (em.dem[indx].data[x0 *sr.ippd + y0]==0)
                                fprintf(fd,"%c%c%c",0,0,170);
                            else
                            {
                                /* Elevation: Greyscale */
                                terrain=(unsigned)(0.5+pow((double)(em.dem[indx].data[x0 *sr.ippd + y0]-em.min_elevation),one_over_gamma)*conversion);
                                fprintf(fd,"%c%c%c",terrain,terrain,terrain);
                            }
                        }
                    }
                }
            }
            
            else
            {
                /* We should never get here, but if */
                /* we do, display the region as black */
                
                fprintf(fd,"%c%c%c",0,0,0);
            }
        }
    }
    
    if (sr.kml==0 && sr.geo==0)
    {
        /* Display legend along bottom of image
         * if not generating .sr.kmlor .geo output.
         */
        
        colorwidth=(int)rint((float)width/(float)region.levels);
        
        for (y0=0; y0<30; y0++)
        {
            for (x0=0; x0<(int)width; x0++)
            {
                indx=x0/colorwidth;
                x=x0%colorwidth;
                level=region.level[indx];
                
                hundreds=level/100;
                
                if (hundreds>0)
                    level-=(hundreds*100);
                
                tens=level/10;
                
                if (tens>0)
                    level-=(tens*10);
                
                units=level;
                
                if (y0>=8 && y0<=23)
                {
                    if (hundreds>0)
                    {
                        if (x>=11 && x<=18)
                            if (fontdata[16*(hundreds+'0')+(y0-8)]&(128>>(x-11)))
                                indx=255;
                    }
                    
                    if (tens>0 || hundreds>0)
                    {
                        if (x>=19 && x<=26)
                            if (fontdata[16*(tens+'0')+(y0-8)]&(128>>(x-19)))
                                indx=255;
                    }
                    
                    if (x>=27 && x<=34)
                        if (fontdata[16*(units+'0')+(y0-8)]&(128>>(x-27)))
                            indx=255;
                    
                    if (x>=42 && x<=49)
                        if (fontdata[16*('d')+(y0-8)]&(128>>(x-42)))
                            indx=255;
                    
                    if (x>=50 && x<=57)
                        if (fontdata[16*('B')+(y0-8)]&(128>>(x-50)))
                            indx=255;
                }
                
                if (indx>region.levels)
                    fprintf(fd,"%c%c%c",0,0,0);
                else
                {
                    red=region.color[indx][0];
                    green=region.color[indx][1];
                    blue=region.color[indx][2];
                    
                    fprintf(fd,"%c%c%c",red,green,blue);
                }
            }
        }
    }
    
    fclose(fd);
    
    
    if (sr.kml)
    {
        /* Write colorkey image file */
        
        fd=fopen(ckfile.c_str(),"wb");
        
        height=30*region.levels;
        width=100;
        
        fprintf(fd,"P6\n%u %u\n255\n",width,height);
        
        for (y0=0; y0<(int)height; y0++)
        {
            for (x0=0; x0<(int)width; x0++)
            {
                indx=y0/30;
                x=x0;
                level=region.level[indx];
                
                hundreds=level/100;
                
                if (hundreds>0)
                    level-=(hundreds*100);
                
                tens=level/10;
                
                if (tens>0)
                    level-=(tens*10);
                
                units=level;
                
                if ((y0%30)>=8 && (y0%30)<=23)
                {
                    if (hundreds>0)
                    {
                        if (x>=11 && x<=18)
                            if (fontdata[16*(hundreds+'0')+((y0%30)-8)]&(128>>(x-11)))
                                indx=255;
                    }
                    
                    if (tens>0 || hundreds>0)
                    {
                        if (x>=19 && x<=26)
                            if (fontdata[16*(tens+'0')+((y0%30)-8)]&(128>>(x-19)))
                                indx=255;
                    }
                    
                    if (x>=27 && x<=34)
                        if (fontdata[16*(units+'0')+((y0%30)-8)]&(128>>(x-27)))
                            indx=255;
                    
                    if (x>=42 && x<=49)
                        if (fontdata[16*('d')+((y0%30)-8)]&(128>>(x-42)))
                            indx=255;
                    
                    if (x>=50 && x<=57)
                        if (fontdata[16*('B')+((y0%30)-8)]&(128>>(x-50)))
                            indx=255;
                }
                
                if (indx>region.levels)
                    fprintf(fd,"%c%c%c",0,0,0);
                else
                {
                    red=region.color[indx][0];
                    green=region.color[indx][1];
                    blue=region.color[indx][2];
                    
                    fprintf(fd,"%c%c%c",red,green,blue);
                }
            }
        }
        
        fclose(fd);
    }
    
    fprintf(stdout,"Done!\n");
    fflush(stdout);
}

void Image::WriteImageSS(Region &region)
{
    /* This function generates a topographic map in Portable Pix Map
     (PPM) format based on the signal strength values held in the
     signal[][] array.  The image created is rotated counter-clockwise
     90 degrees from its representation in em.dem[][] so that north
     points up and east points right in the image generated. */
    
    string basename, mapfile, geofile, kmlfile, ckfile;
    bool found;
    unsigned width, height, terrain, red, green, blue;
    unsigned char mask, cityorcounty;
    int indx, x, y, z=1, x0 = 0, y0 = 0, signal, level, hundreds,
    tens, units, match, colorwidth;
    double conversion, one_over_gamma, lat, lon,
    north, south, east, west, minwest;
    FILE *fd;
    
    one_over_gamma=1.0/GAMMA;
    conversion=255.0/pow((double)(em.max_elevation-em.min_elevation),one_over_gamma);
    
    width=(unsigned)(sr.ippd*Utilities::ReduceAngle(em.max_west-em.min_west));
    height=(unsigned)(sr.ippd*Utilities::ReduceAngle(em.max_north-em.min_north));
    
    region.LoadSignalColors(xmtr[0]);
    
    if (filename.empty())
    {
        basename = Utilities::Basename(xmtr[0].filename);
        filename = basename + ".ppm";
    }
    else
    {
        basename = Utilities::Basename(filename);
    }
    
    mapfile = basename + ".ppm";
    geofile = basename + ".geo";
    kmlfile = basename + ".kml";
    ckfile  = basename + "-ck.ppm";
    
    minwest=((double)em.min_west)+sr.dpp;
    
    if (minwest>360.0)
        minwest-=360.0;
    
    north=(double)em.max_north-sr.dpp;
    
    if (sr.kml|| sr.geo)
        south=(double)em.min_north;	/* No bottom legend */
    else
        south=(double)em.min_north-(30.0/sr.ppd);	/* 30 pixels for bottom legend */
    
    east=(minwest<180.0?-minwest:360.0-em.min_west);
    west=(double)(em.max_west<180?-em.max_west:360-em.max_west);
    
    if (sr.geo && sr.kml==0)
    {
        WriteGeo(geofile, mapfile, north, south, east, west, width, height);
    }
    
    if (sr.kml&& sr.geo==0)
    {
        WriteKmlForImage("SPLAT! Signal Strength Contours", xmtr[0].name + " Transmitter Contours", true, kmlfile, mapfile, north, south, east, west, ckfile);
    }
    
    fd=fopen(mapfile.c_str(),"wb");
    
    if (sr.kml|| sr.geo)
    {
        /* No bottom legend */
        
        fprintf(fd,"P6\n%u %u\n255\n",width,height);
        fprintf(stdout,"\nWriting \"%s\" (%ux%u pixmap image)... ",mapfile.c_str(),width,height);
    }
    
    else
    {
        /* Allow space for bottom legend */
        
        fprintf(fd,"P6\n%u %u\n255\n",width,height+30);
        fprintf(stdout,"\nWriting \"%s\" (%ux%u pixmap image)... ",mapfile.c_str(),width,height+30);
    }
    
    fflush(stdout);
    
    for (y = 0, lat=north; y<(int)height; y++, lat=north-(sr.dpp*(double)y))
    {
        for (x=0, lon=em.max_west; x<(int)width; x++, lon=em.max_west-(sr.dpp*(double)x))
        {
            if (lon<0.0)
                lon+=360.0;
            
            for (indx=0, found = false; indx<sr.maxpages && !found;)
            {
                x0=(int)rint(sr.ppd*(lat-(double)em.dem[indx].min_north));
                y0=sr.mpi-(int)rint(sr.ppd*(Utilities::LonDiff((double)em.dem[indx].max_west,lon)));
                
                if (x0>=0 && x0<=sr.mpi && y0>=0 && y0<=sr.mpi)
                    found = true;
                else
                    indx++;
            }
            
            if (found)
            {
                mask=em.dem[indx].mask[x0 *sr.ippd + y0];
                signal=(em.dem[indx].signal[x0 *sr.ippd + y0])-100;
                cityorcounty=0;
                
                match=255;
                
                red=0;
                green=0;
                blue=0;
                
                if (signal>=region.level[0])
                    match=0;
                else
                {
                    for (z=1; (z<region.levels && match==255); z++)
                    {
                        if (signal<region.level[z-1] && signal>=region.level[z])
                            match=z;
                    }
                }
                
                if (match<region.levels)
                {
                    if (sr.smooth_contours && match>0)
                    {
                        red=(unsigned)Utilities::interpolate(region.color[match][0],region.color[match-1][0],region.level[match],region.level[match-1],signal);
                        green=(unsigned)Utilities::interpolate(region.color[match][1],region.color[match-1][1],region.level[match],region.level[match-1],signal);
                        blue=(unsigned)Utilities::interpolate(region.color[match][2],region.color[match-1][2],region.level[match],region.level[match-1],signal);
                    }
                    
                    else
                    {
                        red=region.color[match][0];
                        green=region.color[match][1];
                        blue=region.color[match][2];
                    }
                }
                
                if (mask&2)
                {
                    /* Text Labels: Red or otherwise */
                    
                    if (red>=180 && green<=75 && blue<=75)
                        fprintf(fd,"%c%c%c",255^red,255^green,255^blue);
                    else
                        fprintf(fd,"%c%c%c",255,0,0);
                    
                    cityorcounty=1;
                }
                
                else if (mask&4)
                {
                    /* County Boundaries: Black */
                    
                    fprintf(fd,"%c%c%c",0,0,0);
                    
                    cityorcounty=1;
                }
                
                if (cityorcounty==0)
                {
                    if (sr.contour_threshold!=0 && signal<sr.contour_threshold)
                    {
                        if (sr.ngs)
                            fprintf(fd,"%c%c%c",255,255,255);
                        else
                        {
                            /* Display land or sea elevation */
                            
                            if (em.dem[indx].data[x0 *sr.ippd + y0]==0)
                                fprintf(fd,"%c%c%c",0,0,170);
                            else
                            {
                                terrain=(unsigned)(0.5+pow((double)(em.dem[indx].data[x0 *sr.ippd + y0]-em.min_elevation),one_over_gamma)*conversion);
                                fprintf(fd,"%c%c%c",terrain,terrain,terrain);
                            }
                        }
                    }
                    
                    else
                    {
                        /* Plot field strength regions in color */
                        
                        if (red!=0 || green!=0 || blue!=0)
                            fprintf(fd,"%c%c%c",red,green,blue);
                        
                        else  /* terrain / sea-level */
                        {
                            if (sr.ngs)
                                fprintf(fd,"%c%c%c",255,255,255);
                            else
                            {
                                if (em.dem[indx].data[x0 *sr.ippd + y0]==0)
                                    fprintf(fd,"%c%c%c",0,0,170);
                                else
                                {
                                    /* Elevation: Greyscale */
                                    terrain=(unsigned)(0.5+pow((double)(em.dem[indx].data[x0 *sr.ippd + y0]-em.min_elevation),one_over_gamma)*conversion);
                                    fprintf(fd,"%c%c%c",terrain,terrain,terrain);
                                }
                            }
                        }
                    }
                }
            }
            
            else
            {
                /* We should never get here, but if */
                /* we do, display the region as black */
                
                fprintf(fd,"%c%c%c",0,0,0);
            }
        }
    }
    
    if (sr.kml==0 && sr.geo==0)
    {
        /* Display legend along bottom of image
         * if not generating .sr.kmlor .geo output.
         */
        
        colorwidth=(int)rint((float)width/(float)region.levels);
        
        for (y0=0; y0<30; y0++)
        {
            for (x0=0; x0<(int)width; x0++)
            {
                indx=x0/colorwidth;
                x=x0%colorwidth;
                level=region.level[indx];
                
                hundreds=level/100;
                
                if (hundreds>0)
                    level-=(hundreds*100);
                
                tens=level/10;
                
                if (tens>0)
                    level-=(tens*10);
                
                units=level;
                
                if (y0>=8 && y0<=23)
                {
                    if (hundreds>0)
                    {
                        if (x>=5 && x<=12)
                            if (fontdata[16*(hundreds+'0')+(y0-8)]&(128>>(x-5)))
                                indx=255;
                    }
                    
                    if (tens>0 || hundreds>0)
                    {
                        if (x>=13 && x<=20)
                            if (fontdata[16*(tens+'0')+(y0-8)]&(128>>(x-13)))
                                indx=255;
                    }
                    
                    if (x>=21 && x<=28)
                        if (fontdata[16*(units+'0')+(y0-8)]&(128>>(x-21)))
                            indx=255;
                    
                    if (x>=36 && x<=43)
                        if (fontdata[16*('d')+(y0-8)]&(128>>(x-36)))
                            indx=255;
                    
                    if (x>=44 && x<=51)
                        if (fontdata[16*('B')+(y0-8)]&(128>>(x-44)))
                            indx=255;
                    
                    if (x>=52 && x<=59)
                        if (fontdata[16*(230)+(y0-8)]&(128>>(x-52)))
                            indx=255;
                    
                    if (x>=60 && x<=67)
                        if (fontdata[16*('V')+(y0-8)]&(128>>(x-60)))
                            indx=255;
                    
                    if (x>=68 && x<=75)
                        if (fontdata[16*('/')+(y0-8)]&(128>>(x-68)))
                            indx=255;
                    
                    if (x>=76 && x<=83)
                        if (fontdata[16*('m')+(y0-8)]&(128>>(x-76)))
                            indx=255;
                }
                
                if (indx>region.levels)
                    fprintf(fd,"%c%c%c",0,0,0);
                else
                {
                    red=region.color[indx][0];
                    green=region.color[indx][1];
                    blue=region.color[indx][2];
                    
                    fprintf(fd,"%c%c%c",red,green,blue);
                }
            }
        }
    }
    
    fclose(fd);
    
    if (sr.kml)
    {
        /* Write colorkey image file */
        
        fd=fopen(ckfile.c_str(),"wb");
        
        height=30*region.levels;
        width=100;
        
        fprintf(fd,"P6\n%u %u\n255\n",width,height);
        
        for (y0=0; y0<(int)height; y0++)
        {
            for (x0=0; x0<(int)width; x0++)
            {
                indx=y0/30;
                x=x0;
                level=region.level[indx];
                
                hundreds=level/100;
                
                if (hundreds>0)
                    level-=(hundreds*100);
                
                tens=level/10;
                
                if (tens>0)
                    level-=(tens*10);
                
                units=level;
                
                if ((y0%30)>=8 && (y0%30)<=23)
                {
                    if (hundreds>0)
                    {
                        if (x>=5 && x<=12)
                            if (fontdata[16*(hundreds+'0')+((y0%30)-8)]&(128>>(x-5)))
                                indx=255;
                    }
                    
                    if (tens>0 || hundreds>0)
                    {
                        if (x>=13 && x<=20)
                            if (fontdata[16*(tens+'0')+((y0%30)-8)]&(128>>(x-13)))
                                indx=255;
                    }
                    
                    if (x>=21 && x<=28)
                        if (fontdata[16*(units+'0')+((y0%30)-8)]&(128>>(x-21)))
                            indx=255;
                    
                    if (x>=36 && x<=43)
                        if (fontdata[16*('d')+((y0%30)-8)]&(128>>(x-36)))
                            indx=255;
                    
                    if (x>=44 && x<=51)
                        if (fontdata[16*('B')+((y0%30)-8)]&(128>>(x-44)))
                            indx=255;
                    
                    if (x>=52 && x<=59)
                        if (fontdata[16*(230)+((y0%30)-8)]&(128>>(x-52)))
                            indx=255;
                    
                    if (x>=60 && x<=67)
                        if (fontdata[16*('V')+((y0%30)-8)]&(128>>(x-60)))
                            indx=255;
                    
                    if (x>=68 && x<=75)
                        if (fontdata[16*('/')+((y0%30)-8)]&(128>>(x-68)))
                            indx=255;
                    
                    if (x>=76 && x<=83)
                        if (fontdata[16*('m')+((y0%30)-8)]&(128>>(x-76)))
                            indx=255;
                }
                
                if (indx>region.levels)
                    fprintf(fd,"%c%c%c",0,0,0);
                else
                {
                    red=region.color[indx][0];
                    green=region.color[indx][1];
                    blue=region.color[indx][2];
                    
                    fprintf(fd,"%c%c%c",red,green,blue);
                }
            }
        }
        
        fclose(fd);
    }
    
    fprintf(stdout,"Done!\n");
    fflush(stdout);
}

void Image::WriteImageDBM(Region &region)
{
    /* This function generates a topographic map in Portable Pix Map
     (PPM) format based on the signal power level values held in the
     signal[][] array.  The image created is rotated counter-clockwise
     90 degrees from its representation in em.dem[][] so that north
     points up and east points right in the image generated. */
    
    string basename, mapfile, geofile, kmlfile, ckfile;
    bool found;
    unsigned width, height, terrain, red, green, blue;
    unsigned char mask, cityorcounty;
    int indx, x, y, z=1, x0 = 0, y0 = 0, dBm, level, hundreds,
    tens, units, match, colorwidth;
    double conversion, one_over_gamma, lat, lon,
    north, south, east, west, minwest;
    FILE *fd;
    
    one_over_gamma=1.0/GAMMA;
    conversion=255.0/pow((double)(em.max_elevation-em.min_elevation),one_over_gamma);
    
    width=(unsigned)(sr.ippd*Utilities::ReduceAngle(em.max_west-em.min_west));
    height=(unsigned)(sr.ippd*Utilities::ReduceAngle(em.max_north-em.min_north));
    
    region.LoadDBMColors(xmtr[0]);
    
    if (filename.empty())
    {
        basename = Utilities::Basename(xmtr[0].filename);
        filename = basename + ".ppm";
    }
    else
    {
        basename = Utilities::Basename(filename);
    }
    
    mapfile = basename + ".ppm";
    geofile = basename + ".geo";
    kmlfile = basename + ".kml";
    ckfile  = basename + "-ck.ppm";
    
    minwest=((double)em.min_west)+sr.dpp;
    
    if (minwest>360.0)
        minwest-=360.0;
    
    north=(double)em.max_north-sr.dpp;
    
    if (sr.kml|| sr.geo)
        south=(double)em.min_north;	/* No bottom legend */
    else
        south=(double)em.min_north-(30.0/sr.ppd);	/* 30 pixels for bottom legend */
    
    east=(minwest<180.0?-minwest:360.0-em.min_west);
    west=(double)(em.max_west<180?-em.max_west:360-em.max_west);
    
    if (sr.geo && sr.kml==0)
    {
        WriteGeo(geofile, mapfile, north, south, east, west, width, height);
    }
    
    if (sr.kml&& sr.geo==0)
    {
        WriteKmlForImage("SPLAT! Signal Power Level Contours", xmtr[0].name + " Transmitter Contours", true, kmlfile, mapfile, north, south, east, west, ckfile);
    }
    
    fd=fopen(mapfile.c_str(),"wb");
    
    if (sr.kml|| sr.geo)
    {
        /* No bottom legend */
        
        fprintf(fd,"P6\n%u %u\n255\n",width,height);
        fprintf(stdout,"\nWriting \"%s\" (%ux%u pixmap image)... ",mapfile.c_str(),width,height);
    }
    
    else
    {
        /* Allow for bottom legend */
        
        fprintf(fd,"P6\n%u %u\n255\n",width,height+30);
        fprintf(stdout,"\nWriting \"%s\" (%ux%u pixmap image)... ",mapfile.c_str(),width,height+30);
    }
    
    fflush(stdout);
    
    for (y = 0, lat=north; y<(int)height; y++, lat=north-(sr.dpp*(double)y))
    {
        for (x=0, lon=em.max_west; x<(int)width; x++, lon=em.max_west-(sr.dpp*(double)x))
        {
            if (lon<0.0)
                lon+=360.0;
            
            for (indx=0, found = false; indx<sr.maxpages && !found;)
            {
                x0=(int)rint(sr.ppd*(lat-(double)em.dem[indx].min_north));
                y0=sr.mpi-(int)rint(sr.ppd*(Utilities::LonDiff((double)em.dem[indx].max_west,lon)));
                
                if (x0>=0 && x0<=sr.mpi && y0>=0 && y0<=sr.mpi)
                    found = true;
                else
                    indx++;
            }
            
            if (found)
            {
                mask=em.dem[indx].mask[x0 *sr.ippd + y0];
                dBm=(em.dem[indx].signal[x0 *sr.ippd + y0])-200;
                cityorcounty=0;
                
                match=255;
                
                red=0;
                green=0;
                blue=0;
                
                if (dBm>=region.level[0])
                    match=0;
                else
                {
                    for (z=1; (z<region.levels && match==255); z++)
                    {
                        if (dBm<region.level[z-1] && dBm>=region.level[z])
                            match=z;
                    }
                }
                
                if (match<region.levels)
                {
                    if (sr.smooth_contours && match>0)
                    {
                        red=(unsigned)Utilities::interpolate(region.color[match][0],region.color[match-1][0],region.level[match],region.level[match-1],dBm);
                        green=(unsigned)Utilities::interpolate(region.color[match][1],region.color[match-1][1],region.level[match],region.level[match-1],dBm);
                        blue=(unsigned)Utilities::interpolate(region.color[match][2],region.color[match-1][2],region.level[match],region.level[match-1],dBm);
                    }
                    
                    else
                    {
                        red=region.color[match][0];
                        green=region.color[match][1];
                        blue=region.color[match][2];
                    }
                }
                
                if (mask&2)
                {
                    /* Text Labels: Red or otherwise */
                    
                    if (red>=180 && green<=75 && blue<=75 && dBm!=0)
                        fprintf(fd,"%c%c%c",255^red,255^green,255^blue);
                    else
                        fprintf(fd,"%c%c%c",255,0,0);
                    
                    cityorcounty=1;
                }
                
                else if (mask&4)
                {
                    /* County Boundaries: Black */
                    
                    fprintf(fd,"%c%c%c",0,0,0);
                    
                    cityorcounty=1;
                }
                
                if (cityorcounty==0)
                {
                    if (sr.contour_threshold!=0 && dBm<sr.contour_threshold)
                    {
                        if (sr.ngs) /* No terrain */
                            fprintf(fd,"%c%c%c",255,255,255);
                        else
                        {
                            /* Display land or sea elevation */
                            
                            if (em.dem[indx].data[x0 *sr.ippd + y0]==0)
                                fprintf(fd,"%c%c%c",0,0,170);
                            else
                            {
                                terrain=(unsigned)(0.5+pow((double)(em.dem[indx].data[x0 *sr.ippd + y0]-em.min_elevation),one_over_gamma)*conversion);
                                fprintf(fd,"%c%c%c",terrain,terrain,terrain);
                            }
                        }
                    }
                    
                    else
                    {
                        /* Plot signal power level regions in color */
                        
                        if (red!=0 || green!=0 || blue!=0)
                            fprintf(fd,"%c%c%c",red,green,blue);
                        
                        else  /* terrain / sea-level */
                        {
                            if (sr.ngs)
                                fprintf(fd,"%c%c%c",255,255,255);
                            else
                            {
                                if (em.dem[indx].data[x0 *sr.ippd + y0]==0)
                                    fprintf(fd,"%c%c%c",0,0,170);
                                else
                                {
                                    /* Elevation: Greyscale */
                                    terrain=(unsigned)(0.5+pow((double)(em.dem[indx].data[x0 *sr.ippd + y0]-em.min_elevation),one_over_gamma)*conversion);
                                    fprintf(fd,"%c%c%c",terrain,terrain,terrain);
                                }
                            }
                        }
                    }
                }
            }
            
            else
            {
                /* We should never get here, but if */
                /* we do, display the region as black */
                
                fprintf(fd,"%c%c%c",0,0,0);
            }
        }
    }
    
    if (sr.kml==0 && sr.geo==0)
    {
        /* Display legend along bottom of image
         if not generating .sr.kmlor .geo output. */
        
        colorwidth=(int)rint((float)width/(float)region.levels);
        
        for (y0=0; y0<30; y0++)
        {
            for (x0=0; x0<(int)width; x0++)
            {
                indx=x0/colorwidth;
                x=x0%colorwidth;
                
                level=abs(region.level[indx]);
                
                hundreds=level/100;
                
                if (hundreds>0)
                    level-=(hundreds*100);
                
                tens=level/10;
                
                if (tens>0)
                    level-=(tens*10);
                
                units=level;
                
                if (y0>=8 && y0<=23)
                {
                    if (hundreds>0)
                    {
                        if (region.level[indx]<0)
                        {
                            if (x>=5 && x<=12)
                                if (fontdata[16*('-')+(y0-8)]&(128>>(x-5)))
                                    indx=255;
                        }
                        
                        else
                        {
                            if (x>=5 && x<=12)
                                if (fontdata[16*('+')+(y0-8)]&(128>>(x-5)))
                                    indx=255;
                        }
                        
                        if (x>=13 && x<=20)
                            if (fontdata[16*(hundreds+'0')+(y0-8)]&(128>>(x-13)))
                                indx=255;
                    }
                    
                    if (tens>0 || hundreds>0)
                    {
                        if (hundreds==0)
                        {
                            if (region.level[indx]<0)
                            {
                                if (x>=13 && x<=20)
                                    if (fontdata[16*('-')+(y0-8)]&(128>>(x-13)))
                                        indx=255;
                            }
                            
                            else
                            {
                                if (x>=13 && x<=20)
                                    if (fontdata[16*('+')+(y0-8)]&(128>>(x-13)))
                                        indx=255;
                            }
                        }
                        
                        if (x>=21 && x<=28)
                            if (fontdata[16*(tens+'0')+(y0-8)]&(128>>(x-21)))
                                indx=255;
                    }
                    
                    if (hundreds==0 && tens==0)
                    {
                        if (region.level[indx]<0)
                        {
                            if (x>=21 && x<=28)
                                if (fontdata[16*('-')+(y0-8)]&(128>>(x-21)))
                                    indx=255;
                        }
                        
                        else
                        {
                            if (x>=21 && x<=28)
                                if (fontdata[16*('+')+(y0-8)]&(128>>(x-21)))
                                    indx=255;
                        }
                    }
                    
                    if (x>=29 && x<=36)
                        if (fontdata[16*(units+'0')+(y0-8)]&(128>>(x-29)))
                            indx=255;
                    
                    if (x>=37 && x<=44)
                        if (fontdata[16*('d')+(y0-8)]&(128>>(x-37)))
                            indx=255;
                    
                    if (x>=45 && x<=52)
                        if (fontdata[16*('B')+(y0-8)]&(128>>(x-45)))
                            indx=255;
                    
                    if (x>=53 && x<=60)
                        if (fontdata[16*('m')+(y0-8)]&(128>>(x-53)))
                            indx=255;
                }
                
                if (indx>region.levels)
                    fprintf(fd,"%c%c%c",0,0,0);
                else
                {
                    red=region.color[indx][0];
                    green=region.color[indx][1];
                    blue=region.color[indx][2];
                    
                    fprintf(fd,"%c%c%c",red,green,blue);
                }
            }
        }
    }
    
    fclose(fd);
    
    
    if (sr.kml)
    {
        /* Write colorkey image file */
        
        fd=fopen(ckfile.c_str(),"wb");
        
        height=30*region.levels;
        width=100;
        
        fprintf(fd,"P6\n%u %u\n255\n",width,height);
        
        for (y0=0; y0<(int)height; y0++)
        {
            for (x0=0; x0<(int)width; x0++)
            {
                indx=y0/30;
                x=x0;
                
                level=abs(region.level[indx]);
                
                hundreds=level/100;
                
                if (hundreds>0)
                    level-=(hundreds*100);
                
                tens=level/10;
                
                if (tens>0)
                    level-=(tens*10);
                
                units=level;
                
                
                if ((y0%30)>=8 && (y0%30)<=23)
                {
                    if (hundreds>0)
                    {
                        if (region.level[indx]<0)
                        {
                            if (x>=5 && x<=12)
                                if (fontdata[16*('-')+((y0%30)-8)]&(128>>(x-5)))
                                    indx=255;
                        }
                        
                        else
                        {
                            if (x>=5 && x<=12)
                                if (fontdata[16*('+')+((y0%30)-8)]&(128>>(x-5)))
                                    indx=255;
                        }
                        
                        if (x>=13 && x<=20)
                            if (fontdata[16*(hundreds+'0')+((y0%30)-8)]&(128>>(x-13)))
                                indx=255;
                    }
                    
                    if (tens>0 || hundreds>0)
                    {
                        if (hundreds==0)
                        {
                            if (region.level[indx]<0)
                            {
                                if (x>=13 && x<=20)
                                    if (fontdata[16*('-')+((y0%30)-8)]&(128>>(x-13)))
                                        indx=255;
                            }
                            
                            else
                            {
                                if (x>=13 && x<=20)
                                    if (fontdata[16*('+')+((y0%30)-8)]&(128>>(x-13)))
                                        indx=255;
                            }
                        }
                        
                        if (x>=21 && x<=28)
                            if (fontdata[16*(tens+'0')+((y0%30)-8)]&(128>>(x-21)))
                                indx=255;
                    }
                    
                    if (hundreds==0 && tens==0)
                    {
                        if (region.level[indx]<0)
                        {
                            if (x>=21 && x<=28)
                                if (fontdata[16*('-')+((y0%30)-8)]&(128>>(x-21)))
                                    indx=255;
                        }
                        
                        else
                        {
                            if (x>=21 && x<=28)
                                if (fontdata[16*('+')+((y0%30)-8)]&(128>>(x-21)))
                                    indx=255;
                        }
                    }
                    
                    if (x>=29 && x<=36)
                        if (fontdata[16*(units+'0')+((y0%30)-8)]&(128>>(x-29)))
                            indx=255;
                    
                    if (x>=37 && x<=44)
                        if (fontdata[16*('d')+((y0%30)-8)]&(128>>(x-37)))
                            indx=255;
                    
                    if (x>=45 && x<=52)
                        if (fontdata[16*('B')+((y0%30)-8)]&(128>>(x-45)))
                            indx=255;
                    
                    if (x>=53 && x<=60)
                        if (fontdata[16*('m')+((y0%30)-8)]&(128>>(x-53)))
                            indx=255;
                }
                
                if (indx>region.levels)
                    fprintf(fd,"%c%c%c",0,0,0);
                
                else
                {
                    red=region.color[indx][0];
                    green=region.color[indx][1];
                    blue=region.color[indx][2];
                    
                    fprintf(fd,"%c%c%c",red,green,blue);
                }
            }
        }
        
        fclose(fd);
    }
    
    fprintf(stdout,"Done!\n");
    fflush(stdout);
}

void Image::WriteKmlForImage(const string &groundOverlayName, const string &description, bool writeScreenOverlay, const string &kmlfile, const string &mapfile, double north, double south, double east, double west, const string &ckfile)
{
    fstream fs;
    fs.open(kmlfile.c_str(), fstream::out);
    
    // TODO: Should we fail silently if we can't open. Shouldn't we WARN?
    if (!fs)
    {
        return;
    }
    
    // Use 5 decimal places.
    ios::fmtflags fsOriginalFlags(fs.flags());
    fs.setf(ios::fixed, ios::floatfield);
    fs.precision(5);
    
    fs << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<sr.kmlxmlns=\"http://earth.google.com/kml/2.1\">\n"
    "<!-- Generated by " << SplatRun::splat_name << " Version " << SplatRun::splat_version << " -->\n"
    "  <Folder>\n"
    "   <name>" << SplatRun::splat_name << "</name>\n"
    "     <description>" << xmtr[0].name << " Transmitter Contours</description>\n"
    "       <GroundOverlay>\n"
    "         <name>" << groundOverlayName << "</name>\n"
    "           <description>" << description << "</description>\n"
    "		<Icon>\n"
    "              <href>" << mapfile << "</href>\n"
    "		</Icon>\n"
    /*       "            <opacity>128</opacity>\n" */
    "            <LatLonBox>\n"
    "               <north>" << north << "</north>\n"
    "               <south>" << south << "</south>\n"
    "               <east>" << east << "</east>\n"
    "               <west>" << west << "</west>\n"
    "               <rotation>0.0</rotation>\n"
    "            </LatLonBox>\n"
    "       </GroundOverlay>\n";
    if (writeScreenOverlay)
    {
        fs << "       <ScreenOverlay>\n"
        "          <name>Color Key</name>\n"
        "            <description>Contour Color Key</description>\n"
        "          <Icon>\n"
        "            <href>" << ckfile << "</href>\n"
        "          </Icon>\n"
        "          <overlayXY x=\"0\" y=\"1\" xunits=\"fraction\" yunits=\"fraction\"/>\n"
        "          <screenXY x=\"0\" y=\"1\" xunits=\"fraction\" yunits=\"fraction\"/>\n"
        "          <rotationXY x=\"0\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>\n"
        "          <size x=\"0\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>\n"
        "       </ScreenOverlay>\n";
    }
    
    // Restore the floating point defaults. TODO: Why use different precision on these floating point numbers?
    fs.flags(fsOriginalFlags);
    
    for (int x=0; x<xmtr.size(); x++)
    {
        fs << "     <Placemark>\n"
        "       <name>" << xmtr[x].name << "</name>\n"
        "       <visibility>1</visibility>\n"
        "       <Style>\n"
        "       <IconStyle>\n"
        "        <Icon>\n"
        "          <href>root://icons/palette-5.png</href>\n"
        "          <x>224</x>\n"
        "          <y>224</y>\n"
        "          <w>32</w>\n"
        "          <h>32</h>\n"
        "        </Icon>\n"
        "       </IconStyle>\n"
        "       </Style>\n"
        "      <Point>\n"
        "        <extrude>1</extrude>\n"
        "        <altituem.demode>relativeToGround</altituem.demode>\n"
        "        <coordinates>" << (xmtr[x].lon<180.0?-xmtr[x].lon:360.0-xmtr[x].lon) << "," << xmtr[x].lat << "," << xmtr[x].alt << "</coordinates>\n"
        "      </Point>\n"
        "     </Placemark>\n";
    }
    
    fs << "  </Folder>\n"
    "</kml>\n";
    
    fs.close();
}

void Image::WriteGeo(const string &geofile, const string &mapfile, double north, double south, double east, double west, unsigned int width, unsigned int height)
{
    fstream fs;
    fs.open(geofile.c_str(), fstream::out);
    
    // TODO: Should we fail silently if we can't open. Shouldn't we WARN?
    if (!fs)
    {
        return;
    }
    
    // Always use 3 decimal places.
    fs.setf(ios::fixed, ios::floatfield);
    fs.precision(3);
    
    fs << "FILENAME\t" << mapfile << "\n"
    "#\t\tX\tY\tLong\t\tLat\n"
    "TIEPOINT\t0\t0\t" << west << "\t\t" << north << "\n"
    "TIEPOINT\t" << width-1 << "\t" << height-1 << "\t" << east << "\t\t" << south << "\n"
    "IMAGESIZE\t" << width << "\t" << height << "\n"
    "#\n# Auto Generated by " << SplatRun::splat_name << " v" << SplatRun::splat_version << "\n#\n";
    
    fs.close();
}
