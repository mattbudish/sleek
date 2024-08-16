#include <filesystem>

#include "codeFile.hpp"
#include "cppParser.hpp"
#include "headerFile.hpp"
#include "project.hpp"
#include "sleekPackage.hpp"
#include "target.hpp"

using namespace std;
namespace fs = std::filesystem;

sleek::codeModel::CodeFile::CodeFile(shared_ptr<Project> pp) : parentProject(pp), hasMain(false)
{}

void sleek::codeModel::CodeFile::scanFile(const string &fileName)
{
    this->fileName = fileName;

    parseFile(
        fileName, 
        [this](const string &fileName, IncludeStyle headerType){
            fs::path filePath { fileName };

            // Search for header file in path relative to this code file, or in current base
            if (headerType == IncludeStyle::Quote)
            {
                filePath = fs::path(this->fileName).parent_path() / fileName;
                // First search relative path.
                if (!fs::directory_entry(filePath).exists())
                {
                    // If that fails, point it to base.
                    filePath = this->parentProject->currentBaseDir / fileName;

                    // Lastly, check the include directory.
                    if (!fs::directory_entry(filePath).exists())
                    {
                        for (auto entry = fs::recursive_directory_iterator(parentProject->options.includeDir);
                            entry != fs::recursive_directory_iterator(); ++entry)
                        {
                            if (entry->path().filename() == fileName)
                            {
                                filePath = entry->path();
                            }
                        }
                    }
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
            }
        }, 
        [this](){
            this->hasMain = true;
            this->parentProject->buildType = BuildType::Exe;
        });
}

// Collect up and merge cflags required for all included headers.
set<string> sleek::codeModel::CodeFile::getCflags() const
{
    set<string> cflags;

    for (auto const &[fName, headerFile] : headerFiles)
    {
        if (nullptr != headerFile->parentPackage)
        {
            cflags.insert(headerFile->parentPackage->cflags.begin(), 
                headerFile->parentPackage->cflags.end());
        }
    }

    return cflags;
}