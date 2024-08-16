#pragma once
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace sleek
{
namespace codeModel
{

// Forward declarations
class Project;
class SleekPackage;

enum class BuildType
{
    Exe,
    Lib,
    H
};

class Target
{
public:
    Target(std::shared_ptr<Project>);
    std::string fileName;
    std::shared_ptr<Project> parentProject;
    BuildType buildType;
    std::map<std::string, std::shared_ptr<CodeFile>> codeFiles;
    std::map<std::string, std::shared_ptr<SleekPackage>> dependencies;
    std::set<std::string> getCflags(void) const;
    std::set<std::string> getLibs(void) const;
};

}
}