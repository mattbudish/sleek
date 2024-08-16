#pragma once
// System Includes
#include <filesystem>
#include <string>
#include <iostream>

#include "codeModel/project.hpp"

// Install Command
namespace sleek
{

namespace command
{

void install(std::shared_ptr<codeModel::Project> project);

}

}