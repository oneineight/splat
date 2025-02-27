/**
 * @file imagewriter.cpp
 *
 * @brief Helper class for writing images.
 *
 * Currently this is one-shot; if you can use it to write one image, and then
 * must dispose of it.
 *
 * General usage:
 *    Instantiate with ImageWriter(const char* filename, ImageType imagetype,
 * int width, int height)
 *
 *      filename: full pathname of file on disk to create
 *      imagetype: one of IMAGETYPE_PPM, IMAGETYPE_PNG, IMAGETYPE_GEOTIFF or IMAGETYPE_JPG
 *      width: width of generated image in pixels
 *      height: height of generated image in pixels
 *
 *    This will throw std:invalid_argument if the file can't be created.
 *
 *    Then add pixels to the current image line AppendPixel(Pixel pixel). If you
 * add more than 'width' pixels (i.e. call it too many times before calling
 * EmitLine()), it will ignore you.
 *
 *    When you've added a line's-worth of data, call EmitLine() to write that
 * line to the file.
 *
 *    If you call EmitLine() more than 'height' times, it will ignore you.
 *
 *    When done, call Finish() to flush everything out to disk and close the
 * file.
 *
 *
 * @author Michel Hoche-Mong
 * Contact: hoche@grok.com
 *
 */

#include "imagewriter.h"
#include <exception>
#include <iostream>
#include <string>

using namespace std;


#define DEFAULT_JPEG_QUALITY 90

ImageWriter::ImageWriter(){}; // private constructor

ImageWriter::ImageWriter(const std::string &filename, ImageType imagetype,
                         int width, int height, double north, double south, double east, double west)
    : m_imagetype(imagetype), m_width(width), m_height(height), m_north(north), m_south(south), m_east(east), m_west(west) {

    m_imgline_signal = new unsigned char[m_width];
    m_imgline_red = new unsigned char[m_width];
    m_imgline_green = new unsigned char[m_width];
    m_imgline_blue = new unsigned char[m_width];
    m_imgline_alpha = new unsigned char[m_width];
    
    m_imgline = new unsigned char[3 * m_width];

    if ((m_fp = fopen(filename.c_str(), "wb")) == NULL) {
        throw std::invalid_argument("Invalid filename");
    }
	
    // XXX TODO: error handling - throw exceptions
    switch (m_imagetype) {
    default:
#ifdef HAVE_LIBPNG
    case IMAGETYPE_PNG:
        m_png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        m_info_ptr = png_create_info_struct(m_png_ptr);
        // write metadata
        m_text_ptr[0].key = strdup("Title");
        m_text_ptr[0].text = strdup("SPLAT!");
        m_text_ptr[0].compression = PNG_TEXT_COMPRESSION_NONE;
        m_text_ptr[1].key = strdup("projection");
        m_text_ptr[1].text = strdup("EPSG:4326");
        m_text_ptr[1].compression = PNG_TEXT_COMPRESSION_NONE;
        m_text_ptr[2].key = strdup("bounds");
        bounds_str = ("[["+to_string(m_south)+","+to_string(m_west)+"],["+to_string(m_north)+","+to_string(m_east)+"]]").c_str();
		sprintf(bounds, "%s", bounds_str.c_str());
        m_text_ptr[2].text = bounds;
        m_text_ptr[2].compression = PNG_TEXT_COMPRESSION_NONE;
        png_set_text(m_png_ptr, m_info_ptr, m_text_ptr, PNG_NTEXT);
        png_init_io(m_png_ptr, m_fp);
        png_set_IHDR(m_png_ptr, m_info_ptr, m_width, m_height,
                     8, /* 8 bits per color or 24 bits per pixel for RGB */
                     PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_BASE);
        png_set_compression_level(m_png_ptr, 6); /* default is Z_DEFAULT_COMPRESSION; see zlib.h */
        png_write_info(m_png_ptr, m_info_ptr);
        break;
#endif
#ifdef HAVE_LIBGDAL
    case IMAGETYPE_GEOTIFF:
		GDALAllRegister();
		papszOptions = CSLSetNameValue(papszOptions, "COMPRESS", "DEFLATE");
		papszOptions = CSLSetNameValue(papszOptions, "TILED", "YES");
		poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
		poDstDS = poDriver->Create(filename.c_str(), m_width, m_height, 5, GDT_Byte, papszOptions);	/* create geotiff file with rgba bands */
		poDstDS->SetGeoTransform(adfGeoTransform);	/* georeferencing of image (see .h file) */
		oSRS.SetWellKnownGeogCS("EPSG:4326");
		oSRS.exportToWkt(&pszSRS_WKT);
		poDstDS->SetProjection(pszSRS_WKT);	/* set projection and spatial reference system*/

		//Add Meta data to the image, see https://gdal.org/drivers/raster/gtiff.html#metadata for the full list.
		poDstDS->SetMetadataItem( "TIFFTAG_DOCUMENTNAME", "Coverage plot" );
		poDstDS->SetMetadataItem( "TIFFTAG_SOFTWARE", "Splat!" );

		CPLFree(pszSRS_WKT);
		
		/* Reprojection NON-FUNCTIONAL... */
		/*poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
		papszOptions = CSLSetNameValue(papszOptions, "COMPRESS", "DEFLATE");
			
		oSRS.SetWellKnownGeogCS("EPSG:3857");
		oSRS.exportToWkt(&pszSRS_WKTproj);
		hTransformArg = GDALCreateGenImgProjTransformer( poDstDS, pszSRS_WKT, NULL, pszSRS_WKTproj, FALSE, 0, 1 );
		GDALSuggestedWarpOutput( poDstDS,GDALGenImgProjTransform, hTransformArg, adfGeoTransform, &nPixels, &nLines );
		GDALDestroyGenImgProjTransformer( hTransformArg );
		poDstDSproj = poDriver->Create("test_12345.tif", nPixels, nLines, 6, GDT_Byte, papszOptions);
		poDstDSproj->SetGeoTransform(adfGeoTransform);
		cout << pszSRS_WKTproj << endl;
		poDstDSproj->SetProjection(pszSRS_WKTproj);
		CPLFree(pszSRS_WKT);
		CPLFree(pszSRS_WKTproj);
		
		psWarpOptions = GDALCreateWarpOptions();
		psWarpOptions->hSrcDS = poDstDS;
		psWarpOptions->hDstDS = poDstDSproj;
		psWarpOptions->nBandCount = 4;
		psWarpOptions->panSrcBands =
			(int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );
		psWarpOptions->panSrcBands[0] = 1;
		psWarpOptions->panSrcBands[1] = 1;
		psWarpOptions->panSrcBands[2] = 1;
		psWarpOptions->panSrcBands[3] = 1;
		psWarpOptions->panDstBands =
			(int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );
		psWarpOptions->panDstBands[0] = 1;
		psWarpOptions->panDstBands[1] = 1;
		psWarpOptions->panDstBands[2] = 1;
		psWarpOptions->panDstBands[3] = 1;
		psWarpOptions->pfnProgress = GDALTermProgress;
		psWarpOptions->pTransformerArg =
			GDALCreateGenImgProjTransformer( poDstDS,
											GDALGetProjectionRef(poDstDS),
											poDstDSproj,
											GDALGetProjectionRef(poDstDSproj),
											FALSE, 0.0, 1 );
		psWarpOptions->pfnTransformer = GDALGenImgProjTransform;
		*/
        break;
#endif
#ifdef HAVE_LIBJPEG
    case IMAGETYPE_JPG:
        m_cinfo.err = jpeg_std_error(&m_jerr);
        jpeg_create_compress(&m_cinfo);
        jpeg_stdio_dest(&m_cinfo, m_fp);
        m_cinfo.image_width = m_width;
        m_cinfo.image_height = m_height;
        m_cinfo.input_components = 3;     /* # of color components per pixel */
        m_cinfo.in_color_space = JCS_RGB; /* colorspace of input image */
        jpeg_set_defaults(&m_cinfo);      /* default compression */
        jpeg_set_quality(&m_cinfo, DEFAULT_JPEG_QUALITY, TRUE);	/* possible range is 0-100 */
        jpeg_start_compress(&m_cinfo, TRUE); /* start compressor. */
        break;
#endif
    case IMAGETYPE_PPM:
        fprintf(m_fp, "P6\n%u %u\n255\n", m_width, m_height);
    }

    m_initialized = true;
};

ImageWriter::~ImageWriter() {
    delete[] m_imgline;

    // close file
    if (m_fp) {
        fclose(m_fp);
    }
};

void ImageWriter::AppendPixel(Pixel pixel) {
	/* populate one image line */
    if (!m_initialized) {
        return;
    }

    if ((m_xoffset + 3) > (m_width * 3)) {
        return;
    }

	/* four distinct lines for red, green, blue and alpha (rgba) for geotiff */
	m_imgline_signal[m_xoffset_rgb] = GetSValue(pixel);
        m_imgline_red[m_xoffset_rgb] = GetRValue(pixel);
	m_imgline_green[m_xoffset_rgb] = GetGValue(pixel);
	m_imgline_blue[m_xoffset_rgb] = GetBValue(pixel);
	m_imgline_alpha[m_xoffset_rgb] = ((pixel & 0xFF00FFFF) == 0xFF00FFFF) ? 0 : 255;	// Select all white pixels and mask them as transparent
	
	m_xoffset_rgb++;
	
	/* 3-byte array (rgb) for other image types */
    m_imgline[m_xoffset++] = GetRValue(pixel);
    m_imgline[m_xoffset++] = GetGValue(pixel);
    m_imgline[m_xoffset++] = GetBValue(pixel);
};

void ImageWriter::EmitLine() {
    if (!m_initialized) {
        return;
    }
    
    if (m_linenumber > m_height) {
		return;
	}

    switch (m_imagetype) {
    default:
#ifdef HAVE_LIBPNG
    case IMAGETYPE_PNG:
        png_write_row(m_png_ptr, (png_bytep)(m_imgline));
        break;
#endif
#ifdef HAVE_LIBGDAL
    case IMAGETYPE_GEOTIFF:
		poDstDS->GetRasterBand(1)->RasterIO(GF_Write, 0, m_linenumber, m_width, 1, m_imgline_red, m_width, 1, GDT_Byte, 0, 0);
		poDstDS->GetRasterBand(2)->RasterIO(GF_Write, 0, m_linenumber, m_width, 1, m_imgline_green, m_width, 1, GDT_Byte, 0, 0);
		poDstDS->GetRasterBand(3)->RasterIO(GF_Write, 0, m_linenumber, m_width, 1, m_imgline_blue, m_width, 1, GDT_Byte, 0, 0);
		poDstDS->GetRasterBand(4)->RasterIO(GF_Write, 0, m_linenumber, m_width, 1, m_imgline_alpha, m_width, 1, GDT_Byte, 0, 0);
		poDstDS->GetRasterBand(5)->RasterIO(GF_Write, 0, m_linenumber, m_width, 1, m_imgline_signal, m_width, 1, GDT_Byte, 0, 0);
        break;
#endif
#ifdef HAVE_LIBJPEG
    case IMAGETYPE_JPG:
        jpeg_write_scanlines(&m_cinfo, &m_imgline, 1);
        break;
#endif
    case IMAGETYPE_PPM:
        fwrite(m_imgline, 3, m_width, m_fp);
        break;
    }
    m_xoffset = 0;
    m_xoffset_rgb = 0;
    m_linenumber++;
};

void ImageWriter::Finish() {
    if (!m_initialized) {
        return;
    }

    switch (m_imagetype) {
    default:
#ifdef HAVE_LIBPNG
    case IMAGETYPE_PNG:
        png_write_end(m_png_ptr, m_info_ptr);
        png_destroy_write_struct(&m_png_ptr, &m_info_ptr);
        break;
#endif
#ifdef HAVE_LIBGDAL
    case IMAGETYPE_GEOTIFF:
		/* warping is not functional yet.
		oOperation.Initialize( psWarpOptions );
		oOperation.ChunkAndWarpImage( 0, 0,
									GDALGetRasterXSize( poDstDSproj ),
									GDALGetRasterYSize( poDstDSproj ) );
		GDALDestroyGenImgProjTransformer( psWarpOptions->pTransformerArg );
		GDALDestroyWarpOptions( psWarpOptions );	
		GDALClose( (GDALDatasetH) poDstDSproj );*/
		GDALClose( (GDALDatasetH) poDstDS );
		GDALDestroyDriverManager();
        break;
#endif
#ifdef HAVE_LIBJPEG
    case IMAGETYPE_JPG:
        jpeg_finish_compress(&m_cinfo);
        jpeg_destroy_compress(&m_cinfo);
        break;
#endif
    case IMAGETYPE_PPM:
        /* do nothing */
        ;
    }
};
