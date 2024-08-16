#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "codeFile.hpp"
#include "headerFile.hpp"
#include "project.hpp"
#include "sleekPackage.hpp"
#include "systemHeaders.hpp"
#include "target.hpp"

using namespace std;
using namespace std::filesystem;

using json = nlohmann::json;

shared_ptr<sleek::codeModel::Project> sleek::codeModel::Project::initializeProject(const ProjectOptions &opt)
{
    auto dir = path(".");
    vector<path> codeFolders = { opt.sourceDir, opt.libDir, opt.includeDir };
    vector<string> extensions = { ".cpp", ".cc", ".c", ".C", ".c++", ".cxx" };
    auto project = shared_ptr<Project>(new Project(opt.name));

    project->options = opt;
    
    Target target(project);
    target.fileName = opt.name;

    project->targets.push_back(target);

    project->currentBaseDir = dir;

    // Scan C++ files
    // Only scan files in one of the defined folders or the project base folder.
    for (auto baseFolder = directory_iterator(dir); baseFolder != directory_iterator(); ++baseFolder)    
    {
        // Add any code files in project base folder
        if (any_of(extensions.begin(), extensions.end(), [&](const string &extension){
            return baseFolder->path().extension() == extension;
        }))
        {
            project->addCodeFile(baseFolder->path());
        }

        // Recursively scan for additional code files
        if (baseFolder->is_directory() && any_of(codeFolders.begin(), codeFolders.end(), [&](path folder){ 
                return baseFolder->path() == dir / folder;}))
        {
            project->currentBaseDir = baseFolder->path().lexically_relative(dir);
            
            for (auto entry = recursive_directory_iterator(*baseFolder); entry != recursive_directory_iterator(); ++entry)
            {
                auto relPath = entry->path().lexically_relative(dir);

                if (any_of(extensions.begin(), extensions.end(), [&](const string &extension){
                    return entry->path().extension() == extension;
                }))
                {
                    project->addCodeFile(relPath);
                }
            }
        }
    }

    project->scanProjectHeaders();

    return project;
}

// TODO: This should be done at the target level
void sleek::codeModel::Project::addCodeFile(const string &fileName)
{
    CodeFile codeFile(shared_from_this());

    codeFile.scanFile(fileName);
    codeFile.fileName = fileName;

    this->codeFiles.emplace(fileName, make_shared<CodeFile>(codeFile));
}

void sleek::codeModel::Project::scanProjectHeaders()
{
    for (const auto &[fileName, header] : headerIndex)
    {
        if (nullptr == header->parentPackage || header->parentPackage->isThirdParty)
        {
            ifstream fs(fileName);

            if (fs.is_open())
            {
                header->parentPackage = packageIndex["this"];
                fs.close();
                header->scanFile(fileName);
            }
        }
    }
}

sleek::codeModel::Project::Project(const string &name) : name(name), buildType(BuildType::Lib), version("0.0.0")
{
    auto thisPackage = make_shared<SleekPackage>(SleekPackage {
        name: "this",
        isThirdParty: false
    });

    packageIndex.emplace("this", thisPackage);

    addSystemHeadersToIndex();
    buildPackageIndex();
}

// System headers and packages should not be in the registry and should come
// with Sleek.

// Add system packages to the internal indexes.
void sleek::codeModel::Project::addSystemHeadersToIndex()
{
    auto cppPackage = make_shared<SleekPackage>(SleekPackage {
        name: "libstdc++",
        version: "6.0.28",
        isThirdParty: false,
        system: true
    });

    packageIndex.emplace("libstdc++", cppPackage);

    auto cPackage = make_shared<SleekPackage>(SleekPackage {
        name: "libc",
        version: "2.31.0",
        isThirdParty: false,
        system: true
    });

    packageIndex.emplace("libc", cPackage);

    for(const string &headerName : systemHeaders::libStdCppHeaders)
    {
        auto header = make_shared<HeaderFile>(headerName);
        header->parentPackage = packageIndex["libstdc++"];
        this->headerIndex.emplace(headerName, header);
    }

    for(const string &headerName : systemHeaders::libCHeaders)
    {
        auto header = make_shared<HeaderFile>(headerName);
        header->parentPackage = packageIndex["libc"];
        this->headerIndex.emplace(headerName, header);
    }

    auto pthreadPackage = make_shared<SleekPackage>(SleekPackage {
        name: "libpthread",
        version: "2.31.0",
        libs: { "-pthread" },
        isThirdParty: false,
        system: true
    });

    packageIndex.emplace("libpthread", pthreadPackage);

    this->headerIndex.emplace("future", make_shared<HeaderFile>("future"));
    this->headerIndex["future"]->parentPackage = packageIndex["libpthread"];
}

void sleek::codeModel::Project::buildPackageIndex()
{
    importIndexDefinitions();
}

// Collect and merge all lib flags from all dependencies.
set<string> sleek::codeModel::Project::getLibs() const
{
    set<string> libs;

    for (const auto &[name, codeFile] : codeFiles)
    {
        for (const auto &[name, headerFile] : codeFile->headerFiles)
        {
            if (nullptr != headerFile->parentPackage)
            {
                libs.insert(headerFile->parentPackage->libs.begin(), headerFile->parentPackage->libs.end());
            }
        }
    }

    return libs;
}

void sleek::codeModel::Project::importIndexDefinitions()
{
    try
    {
        auto indexDir = path("/var/lib/sleek/index");

        for (auto entry = directory_iterator(indexDir); entry != directory_iterator(); ++entry)
        {
            if (entry->is_regular_file() && entry->path().extension() == ".json")
            {
                ifstream inFile = entry->path();
                json j;
                auto package = make_shared<SleekPackage>();

                inFile >> j;

                package->name = j.at("name");
                package->homePage = j.at("homePage");
                package->version = j.at("versions").at("stable");
                package->source = j.at("source").at("stable").at("url");
                package->system = j.at("system");
                package->isThirdParty = true;

                try
                {                
                    package->pkgConfigName = j.at("pkgConfigName");
                }
                catch(const std::exception& e)
                {
                    package->pkgConfigName = j.at("name");
                }
                

                for (auto cflag : j.at("cflags"))
                {
                    package->cflags.push_back(cflag);
                }
                
                for (auto lib : j.at("libs"))
                {
                    package->libs.push_back(lib);
                }

                for (auto header : j.at("headersProvided"))
                {
                    auto headerFile = make_shared<HeaderFile>(header);
                    headerFile->parentPackage = package;
                    this->headerIndex.emplace(header, headerFile);
                }

                packageIndex.emplace(package->name, package);
            }
        }
    }
    catch (filesystem_error &ex)
    {
        cerr << "-- Package index not found." << endl;
    }
}