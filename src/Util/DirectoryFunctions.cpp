// DirectoryFunctions.cpp

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#  include "win/dirent.h"
#else
#  include <dirent.h>
#endif

#include <algorithm>

#include "DirectoryFunctions.h"

// http://stackoverflow.com/questions/612097/how-can-i-get-a-list-of-files-in-a-directory-using-c-or-c
std::vector<std::string> GetListOfFilesFromDirectory(const std::string& d)
{
    std::vector<std::string> names;

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (d.c_str())) != NULL)
    {
        while ((ent = readdir (dir)) != NULL)
        {
            if (S_ISDIR(ent->d_type))
                continue;
            std::string s(ent->d_name);
            names.push_back(s);
        }
        closedir(dir);
    }
    else
    {
        // could not open directory
        perror ("");
    }

    std::sort(names.begin(), names.end());
    return names;
}

std::vector<std::string> GetListOfFilesFromDirectoryAndSubdirs(const std::string& d)
{
    std::vector<std::string> names;
    std::vector<std::string> directories;

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (d.c_str())) != NULL)
    {
        while ((ent = readdir (dir)) != NULL)
        {
            std::string s(ent->d_name);
            if (!s.compare("."))
                continue;
            if (!s.compare(".."))
                continue;
            if (S_ISDIR(ent->d_type))
            {
                directories.push_back(s);
                continue;
            }
            names.push_back(s);
        }
        closedir(dir);
    }
    else
    {
        // could not open directory
        perror ("");
    }

    // Scan all subdirectories(only one level deep, not recursion)
    for (std::vector<std::string>::const_iterator it = directories.begin();
        it != directories.end();
        ++it)
    {
        const std::string& subdir = *it;
        std::vector<std::string> dirFiles = GetListOfFilesFromDirectory(d + subdir);
        for (std::vector<std::string>::const_iterator it = dirFiles.begin();
            it != dirFiles.end();
            ++it)
        {
            const std::string& f = *it;
            const std::string filepath = subdir + "/" + f;
            names.push_back(filepath);
        }
    }

    std::sort(names.begin(), names.end());
    return names;
}
