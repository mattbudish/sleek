// codeModel.cpp
//
// The plan is to build a complete list of system header files. If a header file is not a system file
// scan the project for it, if it is not defined in this project, it belongs to a a package. If the 
// package is not found, install it.

#include <fstream>
#include <filesystem>

#include "cppParser.hpp"
#include "headerFile.hpp"
#include "project.hpp"
#include "sleekPackage.hpp"

using namespace std;
namespace fs = std::filesystem;

sleek::codeModel::HeaderFile::HeaderFile(shared_ptr<Project> pp, const string &fn) : parentProject(pp), fileName(fn)
{}

sleek::codeModel::HeaderFile::HeaderFile(const string &fn) : fileName(fn)
{}

void sleek::codeModel::HeaderFile::scanFile(const string &fileName)
{
    this->fileName = fileName;

    function<void(const string &, IncludeStyle)> headerCb = 
        [this](const string &fileName, IncludeStyle style){
        fs::path filePath { fileName };
        
        // Search for header file in path relative to this code file, or in current base
        if (style == IncludeStyle::Quote)
        {
            filePath = fs::path(this->fileName).parent_path() / fileName;
            // First search relative path.
            if (!fs::directory_entry(filePath).exists())
            {
                // If that fails, point it to base.
                filePath = this->parentProject->currentBaseDir / fileName;
            }
        }

        auto it = this->parentProject->headerIndex.find(filePath);
        if (it != this->parentProject->headerIndex.end())
        {
            this->headerFiles.insert(*it);

            auto package = it->second->parentPackage;
            if (nullptr != package && package->isThirdParty)
            {
                this->parentProject->dependencies.insert({ package->name, package });
            }
        }
        else
        {
            auto header = make_shared<HeaderFile>(parentProject, filePath);
            this->parentProject->headerIndex.insert({ filePath, header });
            this->headerFiles.insert({ filePath, header });
            this->scanFile(filePath);
        }
    };

    parseFile(
        fileName, 
        headerCb, 
        [](){}
    );
}