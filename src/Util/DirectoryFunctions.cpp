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

    // Get a list of files in the directory
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (d.c_str())) != NULL)
    {
        printf("__List of shaders in %s:__\n", d.c_str());

        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL)
        {
#if 0
            // No lstat on Windows...
            struct stat st;
            lstat(ent->d_name, &st);
            if (S_ISDIR(st.st_mode)
                continue;
#endif

            std::string s(ent->d_name);
            if (s.length() < 5)
                continue;

            printf ("  %s\n", s.c_str());
            names.push_back(s);
        }
        closedir (dir);
    }
    else
    {
        /* could not open directory */
        perror ("");
        //return EXIT_FAILURE;
    }

    std::sort(names.begin(), names.end());
    return names;
}
