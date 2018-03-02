#include <iostream>     // std::cout
#include <functional>   // std::mem_fn
#include <vector>
#include <sys/stat.h>
#include <windows.h>
#include <io.h>
#include <regex>
#include <cstdlib>
#include <queue>

BOOL IsFileExist(const std::string& path)
{
    std::string path_ = path;
	char* pathAct = const_cast<char*>(path.c_str());
    DWORD dwAttrib = GetFileAttributes(pathAct);
    return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

BOOL IsDirExist(const std::string& path)
{
    std::string path_ = path;
	char* pathAct = const_cast<char*>(path.c_str());
    DWORD dwAttrib = GetFileAttributes(pathAct);
    return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 != (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

void finder(const std::string& path) {
    bool done = true;
    std::string path_;
    path_ = (path[path.length() - 1] == '\\') ? (path + "*.*") : (path + "\\*.*");
    WIN32_FIND_DATA fd;
    char* pathAct = const_cast<char*>(path_.c_str());
    HANDLE hFind = FindFirstFile(pathAct, &fd);
    while(done) {
        if(std::string(fd.cFileName) != "." && std::string(fd.cFileName) != "..")
            std::cout << std::string(fd.cFileName) << std::endl;
        done = FindNextFile(hFind, &fd);
    }
    return ;
}

std::string filter(const std::string& toFilter) {
    if(toFilter.length() == 0) return toFilter;
    std::string result = "";
    auto index = toFilter.begin();
    while(index != toFilter.end()) {
        if(*index == '^' || *index == '$' || *index == '*' || *index == '+' ||
           *index == '?' || *index == '(' || *index == ')' || *index == '{' ||
           *index == '}' || *index == '.' || *index == '[' || *index == ']') {
            result = result + "\\" + (*index);
           }
        else {
            result = result + (*index);
        }
        index++;
    }
    return result;
}