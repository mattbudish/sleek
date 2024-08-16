#pragma once
// System Includes
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <filesystem>
#include <stdlib.h>
#include <string>

#include "codeModel/project.hpp"

// Install Command
namespace sleek
{

namespace command
{

void uninstall(std::shared_ptr<codeModel::Project> project)
{
    namespace fs = std::filesystem;
    std::string name = fs::current_path().filename();
    std::string install_location;
    
    // Delete file in /usr/local/bin.
    install_location = "/usr/local/bin/" + name;

    if (remove(install_location.c_str()) == 0)
    {
        puts((install_location + " has been successfully uninstalled.").c_str());
    }
    else
    {
        perror("Failure to uninstall!");
    }
}

}

}