#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace sleek
{
namespace codeModel
{

// Forward declarations
class Project;
class SleekPackage;

// HeaderFile links header files to packages.
class HeaderFile
{
public:
    HeaderFile(std::shared_ptr<Project>, const std::string&);
    HeaderFile(const std::string &);
    std::string fileName;
    std::shared_ptr<Project> parentProject;
    std::shared_ptr<SleekPackage> parentPackage;
    std::map<const std::string, std::shared_ptr<HeaderFile>> headerFiles;
    void scanFile(const std::string &fileName);
};

}
}