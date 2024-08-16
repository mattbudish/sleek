#pragma once
#include <map>
#include <memory>
#include <set>
#include <string>

namespace sleek
{
namespace codeModel
{

// Forward declarations
class HeaderFile;
class Project;

class CodeFile
{
public:
    CodeFile(std::shared_ptr<Project>);
    std::string fileName;
    std::shared_ptr<Project> parentProject;
    bool hasMain;
    std::map<const std::string, std::shared_ptr<HeaderFile>> headerFiles;
    void scanFile(const std::string &fileName);
    std::set<std::string> getCflags(void) const;
};

}
}