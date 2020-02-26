/**
 * @file imagewriter.cpp
 *
 * @brief Helper class for writing images.
 * 
 * Currently this is one-shot; if you can use it to write one image, and then must dispose of it.
 *
 * General usage: 
 *    Instantiate with ImageWriter(const char* filename, ImageType imagetype, int width, int height)
 *
 *      filename: full pathname of file on disk to create
 *      imagetype: one of IMAGETYPE_PPM, IMAGETYPE_PNG, or IMAGETYPE_JPG
 *      width: width of generated image in pixels
 *      height: height of generated image in pixels
 *
 *    This will throw std:invalid_argument if the file can't be created.
 *
 *    Then add pixels to the current image line AppendPixel(Pixel pixel). If you add more than 'width'
 *    pixels (i.e. call it too many times before calling EmitLine()), it will ignore you.
 *
 *    When you've added a line's-worth of data, call EmitLine() to write that line to the file.
 *
 *    If you call EmitLine() more than 'height' times, the results are undefined. // XXX fixme
 *
 *    When done, call Finish() to flush everything out to disk and close the file.
 * 
 *
 * @author Michel Hoche-Mong
 * Contact: hoche@grok.com
 *
 */

#include <iostream>
#include <exception>
#include "imagewriter.h"

// XXX HAVE_LIBPNG/HAVE_LIBJPG should be set by the CMakefiles
#define HAVE_LIBPNG

#ifndef _WIN32
#define HAVE_LIBJPEG
#endif

#ifdef HAVE_LIBPNG
#include <png.h>
#endif
#ifdef HAVE_LIBJPEG
extern "C" {
#include "jpeglib.h"
}
#endif

#ifndef _WIN32
#define GetRValue(RGBColor) (uint8_t) (RGBColor & 0xff)
#define GetGValue(RGBColor) (uint8_t) ((((uint32_t)(RGBColor)) >> 8) & 0xff)
#define GetBValue(RGBColor) (uint8_t) ((((uint32_t)(RGBColor)) >> 16) & 0xff)
#endif

typedef uint32_t Pixel;

#define DEFAULT_JPEG_QUALITY 90

ImageWriter::ImageWriter() {}; // private constructor

ImageWriter::ImageWriter(const char* filename, ImageType imagetype, int width, int height) :
        m_imagetype(imagetype),
        m_width(width),
        m_height(height)
{
    m_imgline = new unsigned char[3 * m_width];

    if ( (m_fp=fopen(filename,"wb")) == NULL) {
        throw std::invalid_argument("Invalid filename");
    }

    // XXX TODO: error handling - throw exceptions
    switch (m_imagetype) {
        default:
#ifdef HAVE_LIBPNG
        case IMAGETYPE_PNG:
            m_png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
            m_info_ptr = png_create_info_struct(m_png_ptr);
            png_init_io(m_png_ptr, m_fp);
            png_set_IHDR(m_png_ptr, m_info_ptr,
                    m_width, m_height,
                    8, /* 8 bits per color or 24 bits per pixel for RGB */
                    PNG_COLOR_TYPE_RGB,
                    PNG_INTERLACE_NONE,
                    PNG_COMPRESSION_TYPE_DEFAULT,
                    PNG_FILTER_TYPE_BASE);
            png_set_compression_level(m_png_ptr, 6);  /* default is Z_DEFAULT_COMPRESSION; see zlib.h */
            png_write_info(m_png_ptr, m_info_ptr);
            break;
#endif
#ifdef HAVE_LIBJPEG
        case IMAGETYPE_JPG:
            m_cinfo.err = jpeg_std_error(&m_jerr);
            jpeg_create_compress(&m_cinfo);
            jpeg_stdio_dest(&m_cinfo, m_fp);
            m_cinfo.image_width = m_width;
            m_cinfo.image_height = m_height;
            m_cinfo.input_components = 3;       /* # of color components per pixel */
            m_cinfo.in_color_space = JCS_RGB;   /* colorspace of input image */
            jpeg_set_defaults(&m_cinfo); /* default compression */
            jpeg_set_quality(&m_cinfo, DEFAULT_JPEG_QUALITY, TRUE); /* possible range is 0-100 */
            jpeg_start_compress(&m_cinfo, TRUE); /* start compressor. */
            break;
#endif
        case IMAGETYPE_PPM:
            fprintf(m_fp,"P6\n%u %u\n255\n",m_width,m_height);
    }

    m_initialized = true;
};

ImageWriter::~ImageWriter()
{
    delete[] m_imgline;

    // close file
    if (m_fp) {
        fclose(m_fp);
    }
};

void
ImageWriter::AppendPixel(Pixel pixel)
{
    if (!m_initialized) { 
        return;
    }

    if ((m_xoffset+3) > (m_width*3)) {
        return;
    }

    m_imgline[m_xoffset++]=GetRValue(pixel);
    m_imgline[m_xoffset++]=GetGValue(pixel);
    m_imgline[m_xoffset++]=GetBValue(pixel);
};

void
ImageWriter::EmitLine()
{
    if (!m_initialized) { 
        return;
    }

    switch (m_imagetype) {
        default:
#ifdef HAVE_LIBPNG
        case IMAGETYPE_PNG:
            png_write_row(m_png_ptr, (png_bytep)(m_imgline));
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
};

void
ImageWriter::Finish()
{
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
