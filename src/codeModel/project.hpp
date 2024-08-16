#pragma once
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "codeFile.hpp"

namespace sleek
{
namespace codeModel
{

// Forward declarations
class SleekPackage;
class HeaderFile;
class CodeFile;
class Target;

enum class BuildType;

class ProjectOptions
{
public:
    std::string name;
    bool globalInstall;
    std::string sourceDir;
    std::string buildDir;
    std::string libDir;
    std::string includeDir;
    std::string selectedTarget;
    std::vector<std::string> packageNames;
    std::function<void(std::shared_ptr<Project>)> command;
};

// Project Class
// Internal representation of data required to build the project.
class Project : public std::enable_shared_from_this<Project>
{
public:
    // Initialize the an instance of Project class.
    [[nodiscard]] static std::shared_ptr<Project> initializeProject(const ProjectOptions &opt);
    ProjectOptions options;
    std::string name;
    std::string version;
    std::map<std::string, std::shared_ptr<SleekPackage>> packageIndex;
    std::map<std::string, std::shared_ptr<HeaderFile>> headerIndex;
    std::vector<Target> targets;
    std::map<std::string, std::shared_ptr<CodeFile>> codeFiles;
    BuildType buildType;
    std::map<std::string, std::shared_ptr<SleekPackage>> dependencies;
    std::filesystem::path currentBaseDir;
    void addCodeFile(const std::string &fileName);
    void scanProjectHeaders(void);
    std::set<std::string> getLibs(void) const;

private:
    Project(const std::string &name);
    void addSystemHeadersToIndex(void);
    void buildPackageIndex(void);
    void importIndexDefinitions(void);
};

}
}
