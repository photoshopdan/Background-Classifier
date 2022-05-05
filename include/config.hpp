#ifndef CONFIG_H
#define CONFIG_H

#include <filesystem>
#include "tinyxml2.h"

class Config
{
public:
	Config(std::filesystem::path filename);

	int min_pixel_value() const;
	double white_bg_threshold() const;

private:
	int m_min_pixel_value{};
	double m_white_bg_threshold{};
};

#endif