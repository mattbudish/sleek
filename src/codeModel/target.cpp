#include "project.hpp"
#include "target.hpp"

using namespace std;

sleek::codeModel::Target::Target(shared_ptr<Project> pp) : parentProject(pp), buildType(BuildType::Lib)
{}

