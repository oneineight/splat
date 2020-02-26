/** @file sdf.cpp
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
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <bzlib.h>
#include "site.h"
#include "path.h"
#include "dem.h"
#include "pat_file.h"
#include "splat_run.h"
#include "lrp.h"
#include "sdf.h"
#include "elevation_map.h"

using namespace std;

#define BZBUFFER 65536



int Sdf::LoadSDF_SDF(const string &name, ElevationMap &em)
{
    /* This function reads uncompressed SPLAT Data Files (.sdf)
     containing digital elevation model data into memory.
     Elevation data, maximum and minimum elevations, and
     quadrangle limits are stored in the first available
     em.dem[] structure. */
    
    int	x, y, data, indx, minlat, minlon, maxlat, maxlon;
    char	found, free_page=0, line[20], sdf_file[255],
    path_plus_name[512];
    FILE	*fd;
    
    for (x=0; name[x]!='.' && name[x]!=0 && x<250; x++)
        sdf_file[x]=name[x];
    
    sdf_file[x]=0;
    
    /* Parse filename for minimum latitude and longitude values */
    
    sscanf(sdf_file,"%d:%d:%d:%d",&minlat,&maxlat,&minlon,&maxlon);
    
    sdf_file[x]='.';
    sdf_file[x+1]='s';
    sdf_file[x+2]='d';
    sdf_file[x+3]='f';
    sdf_file[x+4]=0;
    
    /* Is it already in memory? */
    
    for (indx=0, found=0; indx<sr.maxpages && found==0; indx++)
    {
        if (minlat==em.dem[indx].min_north && minlon==em.dem[indx].min_west && maxlat==em.dem[indx].max_north && maxlon==em.dem[indx].max_west)
            found=1;
    }
    
    /* Is room available to load it? */
    
    if (found==0)
    {
        for (indx=0, free_page=0; indx<sr.maxpages && free_page==0; indx++)
            if (em.dem[indx].max_north==-90)
                free_page=1;
    }
    
    indx--;
    
    if (free_page && found==0 && indx>=0 && indx<sr.maxpages)
    {
        /* Search for SDF file in current working directory first */
        
        strncpy(path_plus_name,sdf_file,255);
        
        fd=fopen(path_plus_name,"rb");
        
        if (fd==NULL)
        {
            /* Next, try loading SDF file from path specified
             in $HOME/.splat_path file or by -d argument */
            
            strncpy(path_plus_name,sdf_path.c_str(),255);
            strncat(path_plus_name,sdf_file,254);
            
            fd=fopen(path_plus_name,"rb");
        }
        
        if (fd!=NULL)
        {
            fprintf(stdout,"Loading \"%s\" into page %d...",path_plus_name,indx+1);
            fflush(stdout);
            
            fgets(line,19,fd);
            sscanf(line,"%d",&em.dem[indx].max_west);
            
            fgets(line,19,fd);
            sscanf(line,"%d",&em.dem[indx].min_north);
            
            fgets(line,19,fd);
            sscanf(line,"%d",&em.dem[indx].min_west);
            
            fgets(line,19,fd);
            sscanf(line,"%d",&em.dem[indx].max_north);
            
            for (x=0; x<sr.ippd; x++)
                for (y=0; y<sr.ippd; y++)
                {
                    fgets(line,19,fd);
                    data=atoi(line);
                    
                    em.dem[indx].data[x * sr.ippd + y]=data;
                    em.dem[indx].signal[x * sr.ippd + y]=0;
                    em.dem[indx].mask[x * sr.ippd + y]=0;
                    
                    if (data>em.dem[indx].max_el)
                        em.dem[indx].max_el=data;
                    
                    if (data<em.dem[indx].min_el)
                        em.dem[indx].min_el=data;
                }
            
            fclose(fd);
            
            if (em.dem[indx].min_el<em.min_elevation)
                em.min_elevation=em.dem[indx].min_el;
            
            if (em.dem[indx].max_el>em.max_elevation)
                em.max_elevation=em.dem[indx].max_el;
            
            if (em.max_north==-90)
                em.max_north=em.dem[indx].max_north;
            
            else if (em.dem[indx].max_north>em.max_north)
                em.max_north=em.dem[indx].max_north;
            
            if (em.min_north==90)
                em.min_north=em.dem[indx].min_north;
            
            else if (em.dem[indx].min_north<em.min_north)
                em.min_north=em.dem[indx].min_north;
            
            if (em.max_west==-1)
                em.max_west=em.dem[indx].max_west;
            
            else
            {
                if (abs(em.dem[indx].max_west-em.max_west)<180)
                {
                    if (em.dem[indx].max_west>em.max_west)
                        em.max_west=em.dem[indx].max_west;
                }
                
                else
                {
                    if (em.dem[indx].max_west<em.max_west)
                        em.max_west=em.dem[indx].max_west;
                }
            }
            
            if (em.min_west==360)
                em.min_west=em.dem[indx].min_west;
            
            else
            {
                if (fabs((float) em.dem[indx].min_west-em.min_west)<180.0)
                {
                    if (em.dem[indx].min_west<em.min_west)
                        em.min_west=em.dem[indx].min_west;
                }
                
                else
                {
                    if (em.dem[indx].min_west>em.min_west)
                        em.min_west=em.dem[indx].min_west;
                }
            }
            
            fprintf(stdout," Done!\n");
            fflush(stdout);
            
            return 1;
        }
        
        else
            return -1;
    }
    
    else
        return 0;
}

int Sdf::LoadSDF_BZ(const string &name, ElevationMap &em)
{
    /* This function reads .bz2 compressed SPLAT Data Files containing
     digital elevation model data into memory.  Elevation data,
     maximum and minimum elevations, and quadrangle limits are
     stored in the first available em.dem[] structure. */
    
    int	x, y, data, indx, minlat, minlon, maxlat, maxlon;
    char	found, free_page=0, sdf_file[255], path_plus_name[512],
    *string;
    FILE	*fd;
    BZFILE	*bzfd;
    
    for (x=0; name[x]!='.' && name[x]!=0 && x<247; x++)
        sdf_file[x]=name[x];
    
    sdf_file[x]=0;
    
    /* Parse sdf_file name for minimum latitude and longitude values */
    
    sscanf(sdf_file,"%d:%d:%d:%d",&minlat,&maxlat,&minlon,&maxlon);
    
    sdf_file[x]='.';
    sdf_file[x+1]='s';
    sdf_file[x+2]='d';
    sdf_file[x+3]='f';
    sdf_file[x+4]='.';
    sdf_file[x+5]='b';
    sdf_file[x+6]='z';
    sdf_file[x+7]='2';
    sdf_file[x+8]=0;
    
    /* Is it already in memory? */
    
    for (indx=0, found=0; indx<sr.maxpages && found==0; indx++)
    {
        if (minlat==em.dem[indx].min_north && minlon==em.dem[indx].min_west && maxlat==em.dem[indx].max_north && maxlon==em.dem[indx].max_west)
            found=1;
    }
    
    /* Is room available to load it? */
    
    if (found==0)
    {
        for (indx=0, free_page=0; indx<sr.maxpages && free_page==0; indx++)
            if (em.dem[indx].max_north==-90)
                free_page=1;
    }
    
    indx--;
    
    if (free_page && found==0 && indx>=0 && indx<sr.maxpages)
    {
        /* Search for SDF file in current working directory first */
        
        strncpy(path_plus_name,sdf_file,255);
        
        fd=fopen(path_plus_name,"rb");
        bzfd=BZ2_bzReadOpen(&bzerror,fd,0,0,NULL,0);
        
        if (fd==NULL || bzerror!=BZ_OK)
        {
            /* Next, try loading SDF file from path specified
             in $HOME/.splat_path file or by -d argument */
            
            strncpy(path_plus_name,sdf_path.c_str(),255);
            strncat(path_plus_name,sdf_file,254);
            
            fd=fopen(path_plus_name,"rb");
            bzfd=BZ2_bzReadOpen(&bzerror,fd,0,0,NULL,0);
        }
        
        if (fd!=NULL && bzerror==BZ_OK)
        {
            fprintf(stdout,"Loading \"%s\" into page %d...",path_plus_name,indx+1);
            fflush(stdout);
            
            sscanf(BZfgets(bzfd,255),"%d",&em.dem[indx].max_west);
            sscanf(BZfgets(bzfd,255),"%d",&em.dem[indx].min_north);
            sscanf(BZfgets(bzfd,255),"%d",&em.dem[indx].min_west);
            sscanf(BZfgets(bzfd,255),"%d",&em.dem[indx].max_north);
            
            for (x=0; x<sr.ippd; x++)
                for (y=0; y<sr.ippd; y++)
                {
                    string=BZfgets(bzfd,20);
                    data=atoi(string);
                    
                    em.dem[indx].data[x * sr.ippd + y]=data;
                    em.dem[indx].signal[x * sr.ippd + y]=0;
                    em.dem[indx].mask[x * sr.ippd + y]=0;
                    
                    if (data>em.dem[indx].max_el)
                        em.dem[indx].max_el=data;
                    
                    if (data<em.dem[indx].min_el)
                        em.dem[indx].min_el=data;
                }
            
            fclose(fd);
            
            BZ2_bzReadClose(&bzerror,bzfd);
            
            if (em.dem[indx].min_el<em.min_elevation)
                em.min_elevation=em.dem[indx].min_el;
            
            if (em.dem[indx].max_el>em.max_elevation)
                em.max_elevation=em.dem[indx].max_el;
            
            if (em.max_north==-90)
                em.max_north=em.dem[indx].max_north;
            
            else if (em.dem[indx].max_north>em.max_north)
                em.max_north=em.dem[indx].max_north;
            
            if (em.min_north==90)
                em.min_north=em.dem[indx].min_north;
            
            else if (em.dem[indx].min_north<em.min_north)
                em.min_north=em.dem[indx].min_north;
            
            if (em.max_west==-1)
                em.max_west=em.dem[indx].max_west;
            
            else
            {
                if (abs(em.dem[indx].max_west-em.max_west)<180)
                {
                    if (em.dem[indx].max_west>em.max_west)
                        em.max_west=em.dem[indx].max_west;
                }
                
                else
                {
                    if (em.dem[indx].max_west<em.max_west)
                        em.max_west=em.dem[indx].max_west;
                }
            }
            
            if (em.min_west==360)
                em.min_west=em.dem[indx].min_west;
            
            else
            {
                if (abs(em.dem[indx].min_west-em.min_west)<180)
                {
                    if (em.dem[indx].min_west<em.min_west)
                        em.min_west=em.dem[indx].min_west;
                }
                
                else
                {
                    if (em.dem[indx].min_west>em.min_west)
                        em.min_west=em.dem[indx].min_west;
                }
            }
            
            fprintf(stdout," Done!\n");
            fflush(stdout);
            
            return 1;
        }
        
        else
            return -1;
    }
    
    else
        return 0;
}

char Sdf::LoadSDF(const string &name, ElevationMap &em)
{
    /* This function loads the requested SDF file from the filesystem.
     It first tries to invoke the LoadSDF_SDF() function to load an
     uncompressed SDF file (since uncompressed files load slightly
     faster).  If that attempt fails, then it tries to load a
     compressed SDF file by invoking the LoadSDF_BZ() function.
     If that fails, then we can assume that no elevation data
     exists for the region requested, and that the region
     requested must be entirely over water. */
    
    int	x, y, indx, minlat, minlon, maxlat, maxlon;
    char	found, free_page=0;
    int	return_value=-1;
    
    /* Try to load an uncompressed SDF first. */
    
    return_value=LoadSDF_SDF(name, em);
    
    /* If that fails, try loading a compressed SDF. */
    
    if (return_value==0 || return_value==-1)
        return_value=LoadSDF_BZ(name, em);
    
    /* If neither format can be found, then assume the area is water. */
    
    if (return_value==0 || return_value==-1)
    {
        /* Parse SDF name for minimum latitude and longitude values */
        
        sscanf(name.c_str(),"%d:%d:%d:%d",&minlat,&maxlat,&minlon,&maxlon);
        
        /* Is it already in memory? */
        
        for (indx=0, found=0; indx<sr.maxpages && found==0; indx++)
        {
            if (minlat==em.dem[indx].min_north && minlon==em.dem[indx].min_west && maxlat==em.dem[indx].max_north && maxlon==em.dem[indx].max_west)
                found=1;
        }
        
        /* Is room available to load it? */
        
        if (found==0)
        {
            for (indx=0, free_page=0; indx<sr.maxpages && free_page==0; indx++)
                if (em.dem[indx].max_north==-90)
                    free_page=1;
        }
        
        indx--;
        
        if (free_page && found==0 && indx>=0 && indx<sr.maxpages)
        {
            fprintf(stdout,"Region  \"%s\" assumed as sea-level into page %d...",name.c_str(),indx+1);
            fflush(stdout);
            
            em.dem[indx].max_west=maxlon;
            em.dem[indx].min_north=minlat;
            em.dem[indx].min_west=minlon;
            em.dem[indx].max_north=maxlat;
            
            /* Fill DEM with sea-level topography */
            
            for (x=0; x<sr.ippd; x++)
                for (y=0; y<sr.ippd; y++)
                {
                    em.dem[indx].data[x * sr.ippd + y]=0;
                    em.dem[indx].signal[x * sr.ippd + y]=0;
                    em.dem[indx].mask[x * sr.ippd + y]=0;
                    
                    if (em.dem[indx].min_el>0)
                        em.dem[indx].min_el=0;
                }
            
            if (em.dem[indx].min_el<em.min_elevation)
                em.min_elevation=em.dem[indx].min_el;
            
            if (em.dem[indx].max_el>em.max_elevation)
                em.max_elevation=em.dem[indx].max_el;
            
            if (em.max_north==-90)
                em.max_north=em.dem[indx].max_north;
            
            else if (em.dem[indx].max_north>em.max_north)
                em.max_north=em.dem[indx].max_north;
            
            if (em.min_north==90)
                em.min_north=em.dem[indx].min_north;
            
            else if (em.dem[indx].min_north<em.min_north)
                em.min_north=em.dem[indx].min_north;
            
            if (em.max_west==-1)
                em.max_west=em.dem[indx].max_west;
            
            else
            {
                if (abs(em.dem[indx].max_west-em.max_west)<180)
                {
                    if (em.dem[indx].max_west>em.max_west)
                        em.max_west=em.dem[indx].max_west;
                }
                
                else
                {
                    if (em.dem[indx].max_west<em.max_west)
                        em.max_west=em.dem[indx].max_west;
                }
            }
            
            if (em.min_west==360)
                em.min_west=em.dem[indx].min_west;
            
            else
            {
                if (abs(em.dem[indx].min_west-em.min_west)<180)
                {
                    if (em.dem[indx].min_west<em.min_west)
                        em.min_west=em.dem[indx].min_west;
                }
                
                else
                {
                    if (em.dem[indx].min_west>em.min_west)
                        em.min_west=em.dem[indx].min_west;
                }
            }
            
            fprintf(stdout," Done!\n");
            fflush(stdout);
            
            return_value=1;
        }
    }
    
    return return_value;
}

char *Sdf::BZfgets(BZFILE *bzfd, unsigned length)
{
    /* This function returns at most one less than 'length' number
     of characters from a bz2 compressed file whose file descriptor
     is pointed to by *bzfd.  In operation, a buffer is filled with
     uncompressed data (size = BZBUFFER), which is then parsed
     and doled out as NULL terminated character strings every time
     this function is invoked.  A NULL string indicates an EOF
     or error condition. */
    
    static int x, y, nBuf;
    static char buffer[BZBUFFER+1], output[BZBUFFER+1];
    bool done = false, opened = false;
    
    if (opened!=1 && bzerror==BZ_OK)
    {
        /* First time through.  Initialize everything! */
        
        x=0;
        y=0;
        nBuf=0;
        opened=1;
        output[0]=0;
    }
    
    do
    {
        if (x==nBuf && bzerror!=BZ_STREAM_END && bzerror==BZ_OK && opened)
        {
            /* Uncompress data into a static buffer */
            
            nBuf=BZ2_bzRead(&bzerror, bzfd, buffer, BZBUFFER);
            buffer[nBuf]=0;
            x=0;
        }
        
        /* Build a string from buffer contents */
        
        output[y]=buffer[x];
        
        if (output[y]=='\n' || output[y]==0 || y==(int)length-1)
        {
            output[y+1]=0;
            done=1;
            y=0;
        }
        
        else
            y++;
        x++;
        
    } while (done==0);
    
    if (output[0]==0)
        opened=0;
    
    return (output);
}

