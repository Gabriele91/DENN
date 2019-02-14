#pragma once
#include <string>
#include <vector>

namespace Denn
{
namespace Filesystem
{
    std::string working_dir();
    std::string home_dir();

    bool is_directory(const std::string& directorypath);
    bool is_file(const std::string& filepath);
    bool is_readable(const std::string& filepath);
    bool is_writable(const std::string& filepath);
    bool exists(const std::string& filepath);
    //utilities files
    std::string get_directory(const std::string& filepath);
    std::string get_filename(const std::string& filepath);
    std::string get_basename(const std::string& filepath);
    std::string get_extension(const std::string& filepath);
    std::vector<char> file_read_all(const std::string& filepath);
    std::string text_file_read_all(const std::string& filepath);

    //utilities path
    struct PathOperation
    {
        bool m_success;
        std::string m_path;
    };
    PathOperation get_fullpath(const std::string& relative);
    PathOperation get_relative_to(const std::string& base, const std::string& path);
    //utilities directories
    struct FolderList
    {
        bool m_success;
        std::vector < std::string > m_fields;
    };
    using FilesList       = FolderList;
    using DirectoriesList = FolderList;

    FilesList       get_files(const std::string& directorypath);
    DirectoriesList get_sub_directories(const std::string& directorypath);
}
}
