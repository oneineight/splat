/**
 * @file imagewriter.h
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
 *      imagetype: one of IMAGETYPE_PPM, IMAGETYPE_PNG, or IMAGETYPE_JPG
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
 *    If you call EmitLine() more than 'height' times, the results are
 * undefined. // XXX fixme
 *
 *    When done, call Finish() to flush everything out to disk and close the
 * file.
 *
 *
 * @author Michel Hoche-Mong
 * Contact: hoche@grok.com
 *
 */

#ifndef imagewriter_h
#define imagewriter_h

#include <string>


#ifdef HAVE_LIBPNG
#include <png.h>
#endif
#ifdef HAVE_LIBGDAL
#include <gdal.h>
#endif
#ifdef HAVE_LIBJPEG
extern "C" {
#include "jpeglib.h"
}
#endif

#ifndef _WIN32
#define GetRValue(RGBColor) (uint8_t)(RGBColor & 0xff)
#define GetGValue(RGBColor) (uint8_t)((((uint32_t)(RGBColor)) >> 8) & 0xff)
#define GetBValue(RGBColor) (uint8_t)((((uint32_t)(RGBColor)) >> 16) & 0xff)
#endif

typedef uint32_t Pixel;

typedef enum ImageType {
    IMAGETYPE_PPM = 0,
#ifdef HAVE_LIBPNG
    IMAGETYPE_PNG,
#endif
#ifdef HAVE_LIBJPEG
    IMAGETYPE_JPG,
#endif
#ifdef HAVE_LIBGDAL
    IMAGETYPE_GEOTIFF,
#endif
} ImageType;

class ImageWriter {
  private:
    ImageWriter(); // private constructor

  public:
    explicit ImageWriter(const std::string &filename, ImageType imagetype,
                         int width, int height);
    virtual ~ImageWriter();

    void AppendPixel(Pixel pixel);

    void EmitLine();

    void Finish();

  public:
    bool m_initialized = false;

    FILE *m_fp = NULL;
    ImageType m_imagetype;
    int m_width;
    int m_height;

    int m_xoffset = 0;

    unsigned char *m_imgline = NULL;

#ifdef HAVE_LIBPNG
    png_structp m_png_ptr = NULL;
    png_infop m_info_ptr = NULL;
#endif
#ifdef HAVE_LIBJPEG
    struct jpeg_compress_struct m_cinfo = {0};
    struct jpeg_error_mgr m_jerr = {0};
#endif
};

#endif /* imagewriter.h */
