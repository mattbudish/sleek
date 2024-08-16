#include <algorithm>
#include <iostream>
#include <iterator>

#include "codeModel/headerFile.hpp"
#include "codeModel/project.hpp"
#include "codeModel/sleekPackage.hpp"
#include "codeModel/target.hpp"
#include "build.hpp"

using namespace std;
using namespace std::filesystem;

namespace
{

string joinStrings(const set<string> &strings);

}

void sleek::command::build(shared_ptr<codeModel::Project> project)
{ 
    string projectName = project->options.name;
    path build = project->options.buildDir;
    path dir = ".";
    string objFileStr;
    vector<directory_entry> objFiles;

    for (const auto &[name, codeFile] : project->codeFiles)
    {
        directory_entry codeFileEntry { name };
        auto relPath = codeFileEntry.path().lexically_relative(dir);
        string origExt = codeFileEntry.path().extension();
        directory_entry objFile((build / relPath).replace_extension(origExt + ".o"));
        
        // If the code file or any of the headers from this project that it includes are newer than
        // the object file, re-compile the object file.
        if (!objFile.exists() || (objFile.last_write_time() < codeFileEntry.last_write_time()) ||
            any_of(codeFile->headerFiles.begin(), codeFile->headerFiles.end(),
                [&](pair<string, shared_ptr<sleek::codeModel::HeaderFile>> headerPr){
                    auto header = headerPr.second;
                    return header->parentPackage != nullptr && header->parentPackage->name == "this" && 
                        (objFile.last_write_time() < directory_entry(header->fileName).last_write_time());
                }))
        {
            auto cFlagSet = codeFile->getCflags();
            cFlagSet.emplace("-I.include/");
            cFlagSet.emplace("-I.sleek/include/");
            string cflagStr = joinStrings(cFlagSet);

            create_directories(build / relPath.parent_path());
            // Compile producing list of dependencies
            string command = "g++ -std=gnu++17 -c " + name + " -o " + objFile.path().string() + " " + cflagStr;
            cout << command << endl;
            std::system(command.c_str());
        }

        objFiles.push_back(objFile);
        objFileStr += objFile.path().string() + " ";
    }

    if (project->buildType == codeModel::BuildType::Exe)
    {
        directory_entry target(build / projectName);

        if (!target.exists() || any_of(objFiles.begin(), objFiles.end(), [&](directory_entry objFile){
            return target.last_write_time() < objFile.last_write_time();
        }))
        {
            string libStr = joinStrings(project->getLibs());

            // Link executable
            string command = "g++ " + objFileStr + " -o " + target.path().string() + " " + libStr;
            cout << command << endl;
            std::system(command.c_str());
        }
    }

}

namespace 
{

string joinStrings(const set<string> &strings)
{
    stringstream result;

    copy(strings.begin(), strings.end(), ostream_iterator<string>(result, " "));

    return result.str();
}

}