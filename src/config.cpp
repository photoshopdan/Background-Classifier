#include "config.hpp"

#include <filesystem>
#include "tinyxml2.h"
#include "tinyxml2.cpp"

using namespace tinyxml2;
namespace fs = std::filesystem;

Config::Config(fs::path filename)
{
    const char* default_xml{
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<variables>"
        "<min_pixel_value>230</min_pixel_value>"
        "<white_bg_threshold>0.1</white_bg_threshold>"
        "</variables>" };

    // Load the variables from the XML file. If the XML document cannot be
    // opened or is malformed, revert to default XML, load values and save.
    bool failed{ false };
    XMLDocument xml_doc;
    xml_doc.LoadFile(filename.string().c_str());
    if (xml_doc.ErrorID() == XML_SUCCESS)
    {
        XMLElement* var = xml_doc.FirstChildElement("variables");
        if (var->FirstChildElement("min_pixel_value")
            ->QueryIntText(&m_min_pixel_value) != XML_SUCCESS ||
            var->FirstChildElement("white_bg_threshold")
            ->QueryDoubleText(&m_white_bg_threshold) != XML_SUCCESS)
            failed = true;
    } 
    else
        failed = true;

    if (failed)
    {
        xml_doc.Clear();
        xml_doc.Parse(default_xml);

        XMLElement* var = xml_doc.FirstChildElement("variables");
        var->FirstChildElement("min_pixel_value")
            ->QueryIntText(&m_min_pixel_value);
        var->FirstChildElement("white_bg_threshold")
            ->QueryDoubleText(&m_white_bg_threshold);
        
        xml_doc.SaveFile(filename.string().c_str());
    }
}

int Config::min_pixel_value() const
{
    return m_min_pixel_value;
}

double Config::white_bg_threshold() const
{
    return m_white_bg_threshold;
}
