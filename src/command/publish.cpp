// publish.cpp
#include <filesystem>
#include <iostream>
#include <nlohmann/json.hpp>

#include "codeModel/headerFile.hpp"
#include "codeModel/project.hpp"
#include "codeModel/sleekPackage.hpp"
#include "publish.hpp"

using namespace std;
using namespace std::filesystem;

using json = nlohmann::json;

void buildPackageDefinitions(shared_ptr<sleek::codeModel::Project> project);

void sleek::command::publish(shared_ptr<codeModel::Project> project)
{
    cout << "sleek::command::publish: Publishing project " << project->options.name << endl;

    buildPackageDefinitions(project);

    // json j = {
    //     { "name", projectName }
    // };

    // for (auto &[key, package] : project->dependencies)
    // {
    //     j["dependencies"].push_back({ { "name", key }, { "version", package->version }});
    // }

    // cout << setw(2) << j << endl;
}

void buildPackageDefinitions(shared_ptr<sleek::codeModel::Project> project)
{
    json j;

    for (const auto &[key, sleekPackage] : project->packageIndex)
    {
        if ("this" == key) continue;

        j[key] = {
            { "name", sleekPackage->name },
            { "homePage", sleekPackage->homePage },
            { "versions", {
                { "stable", sleekPackage->version }
            } },
            { "source", {
                { "stable", {
                     { "url", sleekPackage->source } 
                }
            } } },
            { "cflags", sleekPackage->cflags },
            { "libs", sleekPackage->libs },
            { "libsPrivate", sleekPackage->libsPrivate },
            { "system", sleekPackage->system }
        };
    }

    for (const auto &[key, headerFile] : project->headerIndex)
    {
        if (nullptr == headerFile->parentPackage || 
            "this" == headerFile->parentPackage->name)
        {
            continue;
        }

        j[headerFile->parentPackage->name]["headersProvided"]
            .emplace_back(headerFile->fileName);
    }

    cout << setw(4) << j << endl;
}