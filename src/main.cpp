#include <iostream>
#include <chrono>
#include <thread>
#include <filesystem>
#include <vector>
#include <stdexcept>
#include "config.hpp"
#include "jpegparser.hpp"

namespace fs = std::filesystem;

bool validate_args(int argc, char* argv[])
{
    if (argc <= 1)
        return false;

    for (int count{ 1 }; count < argc; count++)
    {
        fs::path input{ argv[count] };
        if (!fs::is_directory(input))
            return false;
    }
    
    return true;
}

void get_jpeg_paths(int argc, char* argv[], std::vector<fs::path>& paths)
{
    for (int root{ 1 }; root < argc; root++)
    {
        for (const auto& branch : fs::recursive_directory_iterator(argv[root]))
        {
            if (fs::is_regular_file(branch.path())
                && (branch.path().extension() == ".jpg"
                    || branch.path().extension() == ".jpeg"
                    || branch.path().extension() == ".JPG"
                    || branch.path().extension() == ".JPEG"))
            {
                paths.push_back(branch.path());
            }
        }
    }
}

void move_file(fs::path file_path, bool white_bg)
{
    fs::path parent_dir;
    if (white_bg)
        parent_dir = file_path.parent_path() /= "WhiteBG";
    else
        parent_dir = file_path.parent_path() /= "OtherBG";

    if (!fs::exists(parent_dir))
        fs::create_directory(parent_dir);

    fs::rename(file_path, parent_dir /= file_path.filename());
}

int main(int argc, char* argv[])
{
    // Confirm at least one directory as been provided, otherwise close.
    if (!validate_args(argc, argv))
    {
        std::cout << "Invalid input. Please drag one or more folders of "
            << "images onto the app.\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        return 0;
    }

    // Recursively search through each directory, storing JPEG file paths.
    std::vector<fs::path> file_paths;
    get_jpeg_paths(argc, argv, file_paths);

    // If no images were found, close.
    if (file_paths.empty())
    {
        std::cout << "No images found.\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        return 0;
    }

    Config config{ "config.xml" };
    JpegParser jpeg_parser{};

    // Open each JPEG file, determine whether the background is white or not,
    // then separate the files into folders.
    for (fs::path file : file_paths)
    {
        double ratio;
        try
        {
            // Obtain the ratio of 'bright' pixels to all pixels.
            ratio = jpeg_parser.get_stats(file, config.min_pixel_value());
        }
        catch (const std::runtime_error& e)
        {
            std::cout << "Error reading " << file.filename().string()
                << ": " << e.what() << '\n';
            continue;
        }

        try
        {
            if (ratio >= config.white_bg_threshold())
                move_file(file, true);
            else
                move_file(file, false);
            std::cout << file.filename().string() << " moved.\n";
        }
        catch (const fs::filesystem_error& e)
        {
            std::cout << "Error moving " << file.filename().string() << ": "
                << e.what() << '\n';
        }
    }

    std::cout << "\nProcess complete, press enter to quit. ";
    std::cin.ignore();
    
    return 0;
}