#pragma once
// System Includes
#include <cstdlib>
#include <filesystem>
#include <string>

#include "codeModel/project.hpp"

// Build command
namespace sleek
{

namespace command
{

void build(std::shared_ptr<codeModel::Project> project);

}

}