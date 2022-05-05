#ifndef JPEGPARSER
#define JPEGPARSER

#include <filesystem>
#include <turbojpeg.h>

class JpegParser
{
public:
	JpegParser();
	~JpegParser();
	JpegParser(const JpegParser&);
	JpegParser(JpegParser&&);
	JpegParser& operator=(const JpegParser&);
	JpegParser& operator=(JpegParser&&);

	// Return the ratio of pixels above a threshold to total pixels.
	// i.e. (number of pixels above min_pixel_value / total number of pixels)
	double get_stats(std::filesystem::path file_path, int min_pixel_value);

private:
	tjhandle m_tj_instance;
	int m_flags;
	int m_pixel_format;
};

#endif
