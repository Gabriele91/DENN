#include "Denn/Core/Filesystem.h"
#include "Denn/Config.h"
#include <sstream>
#include <cstdlib>
#include <cstring>
#ifdef _WIN32
	#include <windows.h>
	#include <direct.h>
	#include <io.h>
	#include <Shlwapi.h>
	#define getcwd _getcwd
	#define access _access
	#define F_OK 00
	#define R_OK 04
	#define W_OK 02
#else
	#include <sys/types.h>
    #include <sys/stat.h>
    #include <dirent.h>
	#include <unistd.h>
#endif

namespace Denn
{
namespace Filesystem
{

    std::string working_dir()
    {
        char buffer[255];
        char *answer = getcwd(buffer, sizeof(buffer));
        if (answer) return answer;
        return "";
    }

    std::string home_dir()
    {
        const char *home_dir = std::getenv("HOME");
        if (home_dir) return home_dir;
        else         return "";
    }

    bool is_file(const std::string& filepath)
    {
        return  access(filepath.c_str(), F_OK) != -1 && !is_directory(filepath);
    }

    bool is_readable(const std::string& filepath)
    {
        return  access(filepath.c_str(), R_OK) != -1;
    }

    bool is_writable(const std::string& filepath)
    {
        return  access(filepath.c_str(), W_OK) != -1;
    }

    bool exists(const std::string& filepath)
    {
        #if _WIN32
            return PathFileExistsA(get_fullpath(filepath).m_path.c_str()) == TRUE;
        #else
            struct stat buf;
            return (stat(filepath.c_str(), &buf) == 0);
        #endif
    }


    //utilities files
    std::string get_directory(const std::string& filepath)
    {
        //do it?
        if (is_directory(filepath)) return filepath;
        //get separetor
        auto separetor = filepath.find_last_of("\\/");
        //test
        if (separetor == std::string::npos)
            return "";
        //return sub path
        return filepath.substr(0, separetor);
    }

    std::string get_filename(const std::string& filepath)
    {
        //copy path
        std::string filename = filepath;
        //get separetor
        auto separetor = filename.find_last_of("\\/");
        //remove rest of path
        if (separetor != std::string::npos) filename.erase(0, separetor + 1);
        //return
        return filename;
    }

    std::string get_basename(const std::string& filepath)
    {
        //copy path
        std::string filename = get_filename(filepath);
        //get dot
        auto dot = filename.rfind(".");
        //extract
        if (dot != std::string::npos) filename.erase(dot);
        //return name
        return filename;
    }

    std::string get_extension(const std::string& filepath)
    {
        //copy path
        std::string filename = get_filename(filepath);
        //get dot
        auto dot = filename.rfind(".");
        //extract
        if (dot != std::string::npos) return filename.substr(dot);
        //return 0
        return "";
    }

    std::vector<char> file_read_all(const std::string& filepath)
    {
        std::vector<char> out;
        /////////////////////////////////////////////////////////////////////
        FILE* file = fopen(filepath.c_str(), "rb");
        //bad case
        if (!file) return out;
        /////////////////////////////////////////////////////////////////////
        std::fseek(file, 0, SEEK_END);
        size_t size = std::ftell(file);
        std::fseek(file, 0, SEEK_SET);
        //no size:
        if(!size) std::fclose(file);
        /////////////////////////////////////////////////////////////////////
        out.resize(size, 0);
        denn_assert_code(std::fread(&out[0], size, 1, file));
        /////////////////////////////////////////////////////////////////////
        std::fclose(file);
        //return
        return out;
    }

    std::string text_file_read_all(const std::string &filepath)
    {
        std::string out;
        /////////////////////////////////////////////////////////////////////
        FILE* file = fopen(filepath.c_str(), "rb");
        //bad case
        if (!file) return "";
        /////////////////////////////////////////////////////////////////////
        std::fseek(file, 0, SEEK_END);
        size_t size = std::ftell(file);
        std::fseek(file, 0, SEEK_SET);
        //no size:
        if(!size) std::fclose(file);
        /////////////////////////////////////////////////////////////////////
        out.resize(size);
        denn_assert_code(std::fread(&out[0], size, 1, file));
        /////////////////////////////////////////////////////////////////////
        std::fclose(file);
        //return
        return out;
    }


    PathOperation get_fullpath(const std::string& relative)
    {
#ifdef _WIN32
        char fullpath[MAX_PATH];
        if (GetFullPathNameA(relative.c_str(), MAX_PATH, fullpath, 0) == 0) return PathOperation{ false, "" };
        return PathOperation{ true, std::string(fullpath) };
#else
        char realname[_POSIX_PATH_MAX];
        if (realpath(relative.c_str(), realname) == 0) return PathOperation{ false, "" };
        return PathOperation{ true, std::string(realname) };
#endif
    }

    std::vector<std::string> split(std::string str, const std::string& delimiter)
    {
        //temps
        size_t pos = 0;
        std::vector<std::string> tokens;
        //search
        while ((pos = str.find(delimiter)) != std::string::npos)
        {
            tokens.push_back(str.substr(0, pos));
            str.erase(0, pos + delimiter.length());
        }
        //add last
        if (str.size()) tokens.push_back(str);
        //return
        return tokens;
    }

    PathOperation get_relative_to(const std::string& base, const std::string& path)
    {
        //get abs paths
        PathOperation absolute_base = get_fullpath(base); if (!absolute_base.m_success) return PathOperation{ false, "" };
        PathOperation absolute_path = get_fullpath(path); if (!absolute_path.m_success) return PathOperation{ false, "" };
#ifdef _WIN32
        char output_path[MAX_PATH];

        if (PathRelativePathToA(output_path,
            absolute_base.m_path.c_str(),
            FILE_ATTRIBUTE_DIRECTORY,
            absolute_path.m_path.c_str(),
            FILE_ATTRIBUTE_NORMAL) == 0) return PathOperation{ false, "" };

        return PathOperation{ true, output_path };
#else
        std::string output_path;

        //split
        std::string separetor("/");
        std::vector< std::string > base_directories = split(absolute_base.m_path, separetor);
        std::vector< std::string > path_directories = split(absolute_path.m_path, separetor);

        //max loop
        size_t max_size = std::min(base_directories.size(), path_directories.size());

        //start index
        size_t i = 0;

        //search dif
        for (i = 0; i != max_size; ++i)
        {
            if (base_directories[i] != path_directories[i]) break;
        }

        //back
        for (size_t j = 0; j < (max_size - i); ++i) output_path += "../";

        //next
        for (i = max_size; i < path_directories.size(); ++i)
        {
            output_path += path_directories[i] + "/";
        }

        //ok
        return PathOperation{ true, output_path };
#endif
    }


#ifdef _WIN32
    bool is_directory(const std::string& directory)
    {
        DWORD ftyp = GetFileAttributesA(directory.c_str());
        if (ftyp == INVALID_FILE_ATTRIBUTES) return false;
        if (ftyp & FILE_ATTRIBUTE_DIRECTORY) return true;
        return false;
    }

    FilesList get_files(const std::string& directorypath)
    {
        //test directory
        if (!is_directory(directorypath)) return FilesList{ false, std::vector<std::string>{} };
        //alloc output
        FilesList output{ true, std::vector<std::string>{} };
        //redefine path
        std::string std_directorypath = directorypath + "/";
        std::string path_all = std_directorypath + "*";
        //attributes
        WIN32_FIND_DATAA ffd;
        //struct stat st;
        HANDLE hFind = FindFirstFileA(path_all.c_str(), &ffd);
        //read all
        do
        {
            if (!std::strcmp(ffd.cFileName, ".")) continue;
            if (!std::strcmp(ffd.cFileName, "..")) continue;
            //put into vector only files
            if (is_file(std_directorypath + ffd.cFileName))
            {
                output.m_fields.push_back(ffd.cFileName);
            }

        } while (FindNextFileA(hFind, &ffd) != 0);
        //return output
        return output;
    }

    DirectoriesList get_sub_directories(const std::string& directorypath)
    {
        //test directory
        if (!is_directory(directorypath)) return DirectoriesList{ false, std::vector<std::string>{} };
        //alloc output
        DirectoriesList output{ true, std::vector<std::string>{} };
        //redefine path
        std::string std_directorypath = directorypath + "/";
        std::string path_all = std_directorypath + "*";
        //attributes
        WIN32_FIND_DATAA ffd;
        //struct stat st;
        HANDLE hFind = FindFirstFileA(path_all.c_str(), &ffd);
        //read all
        do
        {
            if (!std::strcmp(ffd.cFileName, ".")) continue;
            if (!std::strcmp(ffd.cFileName, "..")) continue;
            //put into vector only dirs
            if (is_directory(std_directorypath + ffd.cFileName))
            {
                output.m_fields.push_back(ffd.cFileName);
            }

        } while (FindNextFileA(hFind, &ffd) != 0);
        //return output
        return output;
    }


#else
    bool is_directory(const std::string& directory)
    {
        struct stat st;
        if (stat(directory.c_str(), &st) == 0) return ((st.st_mode & S_IFDIR) != 0);
        return false;
    }

    FilesList get_files(const std::string& directorypath)
    {
        //test directory
        if (!is_directory(directorypath)) return FilesList{ false, std::vector<std::string>{} };
        //attributes
        DIR *directory;
        struct dirent *dirent;
        //open dir
        if ((directory = opendir(directorypath.c_str())) == NULL) return FilesList{ false, std::vector<std::string>{} };
        //alloc output
        FilesList output{ true, std::vector<std::string>{} };
        //redefine path
        std::string std_directorypath = directorypath + "/";
        //read all elements
        while ((dirent = readdir(directory)) != NULL)
        {
            if (!std::strcmp(dirent->d_name, ".")) continue;
            if (!std::strcmp(dirent->d_name, "..")) continue;
            //put into vector only files
            if (is_file(std_directorypath + dirent->d_name))
            {
                output.m_fields.push_back(dirent->d_name);
            }
        }
        //close directory
        closedir(directory);
        //return output
        return output;
    }

    DirectoriesList get_sub_directories(const std::string& directorypath)
    {
        //test directory
        if (!is_directory(directorypath)) return DirectoriesList{ false, std::vector<std::string>{} };
        //attributes
        DIR *directory;
        struct dirent *dirent;
        //open dir
        if ((directory = opendir(directorypath.c_str())) == NULL) return DirectoriesList{ false, std::vector<std::string>{} };
        //alloc output
        DirectoriesList output{ true, std::vector<std::string>{} };
        //redefine path
        std::string std_directorypath = directorypath + "/";
        //read all elements
        while ((dirent = readdir(directory)) != NULL)
        {
            if (!std::strcmp(dirent->d_name, ".")) continue;
            if (!std::strcmp(dirent->d_name, "..")) continue;
            //put into vector only dirs
            if (is_directory(std_directorypath + dirent->d_name))
            {
                output.m_fields.push_back(dirent->d_name);
            }
        }
        //close directory
        closedir(directory);
        //return output
        return output;
    }
#endif


};
}