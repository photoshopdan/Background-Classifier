#include "jpegparser.hpp"
#include <turbojpeg.h>
#include <filesystem>
#include <stdexcept>
#include <fstream>

namespace fs = std::filesystem;

JpegParser::JpegParser()
	: m_tj_instance{ NULL }
	, m_flags{ TJFLAG_FASTDCT | TJFLAG_NOREALLOC }
	, m_pixel_format{ TJPF_GRAY }
{
	if ((m_tj_instance = tjInitDecompress()) == NULL)
		throw std::runtime_error("Failed to launch decompressor instance");
}

JpegParser::~JpegParser()
{
    tjDestroy(m_tj_instance);
}

JpegParser::JpegParser(const JpegParser& parser)
    : m_tj_instance{ NULL }
    , m_flags{ parser.m_flags }
    , m_pixel_format{ parser.m_pixel_format }
{
    if ((m_tj_instance = tjInitDecompress()) == NULL)
        throw std::runtime_error("Failed to launch decompressor instance");
}

JpegParser::JpegParser(JpegParser&& parser)
    : m_tj_instance{ parser.m_tj_instance }
    , m_flags{ parser.m_flags }
    , m_pixel_format{ parser.m_pixel_format }
{
    parser.m_tj_instance = nullptr;
}

JpegParser& JpegParser::operator=(const JpegParser& parser)
{
    if (&parser == this)
        return *this;

    m_flags = parser.m_flags;
    m_pixel_format = parser.m_pixel_format;

    return *this;
}

JpegParser& JpegParser::operator=(JpegParser&& parser)
{
    if (&parser == this)
        return *this;

    m_tj_instance = parser.m_tj_instance;
    m_flags = parser.m_flags;
    m_pixel_format = parser.m_pixel_format;
    parser.m_tj_instance = nullptr;

    return *this;
}

double JpegParser::get_stats(fs::path file_path, int min_pixel_value)
{
    // Get jpeg size and load JPEG file into buffer.
	unsigned long jpeg_size{ 
		static_cast<unsigned long>(fs::file_size(file_path)) };

	std::ifstream jpeg_file{ file_path, std::ios::binary };
	if (!jpeg_file)
		throw std::runtime_error("Could not load file");
    
	unsigned char* jpeg_buf{ NULL };
	if ((jpeg_buf = (unsigned char*)tjAlloc(jpeg_size)) == NULL)
		throw std::runtime_error("Could not allocate jpeg buffer");
	jpeg_file.read(reinterpret_cast<char*>(jpeg_buf), jpeg_size);
	jpeg_file.close();

    // Read file information from header.
    int width;
    int height;
    int subsamp;
    int colorspace;
    if (tjDecompressHeader3(m_tj_instance, jpeg_buf, jpeg_size, &width,
        &height, &subsamp, &colorspace) < 0)
    {
        tjFree(jpeg_buf);
        throw std::runtime_error("Could not decompress header");
    }

    // Decompress the JPEG file as a 1/64th size, grayscale image.
    tjscalingfactor scale{ 1, 8 };
    width = TJSCALED(width, scale);
    height = TJSCALED(height, scale);
    unsigned char* img_buf{ NULL };
    int img_buf_length{ width * height * tjPixelSize[m_pixel_format] };
    if ((img_buf = (unsigned char*)tjAlloc(img_buf_length)) == NULL)
    {
        tjFree(jpeg_buf);
        throw std::runtime_error("Could not allocate image buffer");
    }

    if (tjDecompress2(m_tj_instance, jpeg_buf, jpeg_size, img_buf, width, 0,
        height, m_pixel_format, m_flags) < 0)
    {
        tjFree(jpeg_buf);
        tjFree(img_buf);
        throw std::runtime_error("Could not decompress image");
    }

    // Iterate over array, counting the number of pixels with a value higher
    // than min_pixel_value. Return the ratio of these pixels to all pixels.
    int count{ 0 };
    for (int pixel{ 0 }; pixel < img_buf_length; ++pixel)
    {
        if (img_buf[pixel] >= min_pixel_value)
            count++;
    }

    tjFree(jpeg_buf);
    tjFree(img_buf);
    
    return static_cast<double>(count) / static_cast<double>(img_buf_length);
}