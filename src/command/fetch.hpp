#pragma once
// System Includes
#include <iostream>

// Project Includes
#include "codeModel/project.hpp"

// Fetch Command
namespace sleek
{

namespace command
{

void fetch(std::shared_ptr<codeModel::Project> project);

}

}