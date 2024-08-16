// pack.cpp
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "codeModel/project.hpp"
#include "codeModel/sleekPackage.hpp"
#include "insert.hpp"
#include "pack.hpp"

using namespace std;
namespace fs = std::filesystem;
using json = nlohmann::json;

static void findPackageHeadersProvided(shared_ptr<sleek::codeModel::Project> project);
static void buildPackageDefinition(shared_ptr<sleek::codeModel::Project> project);

namespace
{

vector<string> packageManifest;

}

void sleek::command::pack(shared_ptr<codeModel::Project> project)
{
    string packageFileName = project->name + "-" + project->version + "-x86_64_linux.tar.gz";

    fs::create_directories(fs::path(project->options.buildDir) / "package");

    findPackageHeadersProvided(project);

    buildPackageDefinition(project);

    fs::current_path("build/package");
    archive::insert("../" + packageFileName, packageManifest);
}

void findPackageHeadersProvided(shared_ptr<sleek::codeModel::Project> project)
{
    auto dir = fs::current_path();
    auto buildPackageInclude = dir / project->options.buildDir / "package" / "include";
    vector<string> extensions = { ".hpp", ".hh", ".h", ".H", ".h++", ".hxx" };

    // Scan project for header files
    // Only scan files in the defined include folder or the project base folder.
    for (auto baseFolder = fs::directory_iterator(dir); baseFolder != fs::directory_iterator(); ++baseFolder)    
    {
        // Add any code files in project base folder
        if (any_of(extensions.begin(), extensions.end(), [&](const string &extension){
            return baseFolder->path().extension() == extension;
        }))
        {
            project->packageIndex.at("this")->headersProvided.push_back(baseFolder->path());
            fs::create_directories(buildPackageInclude);
            fs::copy(baseFolder->path(), buildPackageInclude, fs::copy_options::update_existing);
            packageManifest.push_back("include" / baseFolder->path());
        }

        // Recursively scan for additional code files
        if (baseFolder->is_directory() && baseFolder->path() == dir / project->options.includeDir)
        {
            fs::current_path(baseFolder->path());
            
            for (auto entry = fs::recursive_directory_iterator(*baseFolder); entry != fs::recursive_directory_iterator(); ++entry)
            {
                auto relPath = entry->path().lexically_proximate(dir / baseFolder->path());

                if (any_of(extensions.begin(), extensions.end(), [&](const string &extension){
                    return entry->path().extension() == extension;
                }))
                {
                    project->packageIndex.at("this")->headersProvided.push_back(relPath);
                    fs::create_directories(buildPackageInclude / relPath.parent_path().filename());
                    fs::copy(relPath, buildPackageInclude / relPath, fs::copy_options::update_existing);
                    packageManifest.push_back("include" / relPath);
                }
            }

            fs::current_path(dir);
        }
    }
}

void buildPackageDefinition(shared_ptr<sleek::codeModel::Project> project)
{
    string fName = project->options.buildDir + "/package/" + project->name + ".json";
    ofstream packageFile(fName);

    json j = {
        { "name", project->name },
        { "system", false },
        { "versions", {
                { "latest", project->version }
        } }
    };

    for (const auto &header : project->packageIndex.at("this")->headersProvided)
    {
        j["headersProvided"].emplace_back(header);
    }

    for (const auto &[name, package] : project->dependencies)
    {
        j["dependencies"].emplace_back(name);
    }

    cout << "Package Definition:" << endl;
    cout << setw(4) << j << endl;

    packageFile << setw(4) << j << endl;
    packageManifest.push_back(project->name + ".json");
}