// check.cpp
#include <filesystem>
#include <functional>
#include <future>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <unistd.h>

#include "codeModel/codeFile.hpp"
#include "codeModel/headerFile.hpp"
#include "codeModel/project.hpp"
#include "codeModel/sleekPackage.hpp"
#include "codeModel/target.hpp"
#include "extract.hpp"
#include "httpClient.hpp"
#include "fdstream.hpp"
#include "check.hpp"

using namespace std;
using namespace std::filesystem;

static void downloadSource(const string &name, const shared_ptr<sleek::codeModel::SleekPackage> package);
static bool checkPkgConfig(const string &name, shared_ptr<sleek::codeModel::SleekPackage> package);

void sleek::command::check(shared_ptr<codeModel::Project> project)
{
    cout << "sleek::command::check: Initializing project " << project->options.name << endl;

    // Debugging stuff
    function<void(string, const map<const string, shared_ptr<sleek::codeModel::HeaderFile>> &)> 
        print_header_list = [&](
            string indent, 
            const map<const string, shared_ptr<sleek::codeModel::HeaderFile>> &headerFiles)
    {
        for (auto &[key, header_file] : headerFiles)
        {            
            cout << indent << header_file->fileName << endl;
            cout << indent << "  package: ";
            if (nullptr != header_file->parentPackage)
            {
                cout << header_file->parentPackage->name << endl; 
            }
            else
            {
                cout << "?" << endl;
            }

            indent += "  ";
            // Recursively print included headers
            print_header_list(indent, header_file->headerFiles);

            indent.resize(indent.length() - 2);
        }
    };

    for (const auto &[fileName, codeFile] : project->codeFiles)
    {
        cout << fileName << ":" << endl;

        print_header_list("  ", codeFile->headerFiles);

        if (codeFile->hasMain == true)
        {
            cout << "  * This file has a main function." << endl;
        }
    }

    cout << "This project will be built as " 
        << (project->buildType == codeModel::BuildType::Lib ? "a Library." : "an Executable.") << endl;
    cout << "Dependencies:" << endl;
    for (const auto &[name, package] : project->dependencies)
    {
        cout << "  " << name << endl;
        if (checkPkgConfig(name, package))
        {
            cout << "    pkg-config found globally" << endl;
            continue;
        }

        //downloadSource(name, package);
    }
}

void downloadSource(const string &name, const shared_ptr<sleek::codeModel::SleekPackage> package)
{
    regex filename_regex("[^/\\&\?]+\\.\\w{1,4}(?=([\?&].*$|$))");    // Finds filename within URL.
    smatch package_filename;

    if (!regex_search(package->source, package_filename, filename_regex))
    {
        throw runtime_error("Failure to locate file name in: " + package->source);
    }

    if (false == is_directory(status(".sleek/" + name)))
    {
        cout << "    Downloading: " << name << " " << package_filename.str() << endl;

        int filedes[2];

        if (-1 == pipe(filedes))
        {
            throw runtime_error("Failure to open pipe.");
        }

        create_directories(".sleek/" + name);
        current_path(".sleek/" + name);

        future<void> fut = async([](int fd){
            sleek::archive::extract(fd, 1);
        }, filedes[0]);

        sleek::util::ofdstream out_stream { filedes[1] };

        auto http_code = sleek::net::httpClient(package->source, out_stream);
        if (http_code == 200)
        {
            cout << "Succcessfully downloaded " << package->source << endl;
        }
        else
        {
            cout << "Failure to download " << package->source << " HTTP code: " << http_code << endl;
        }
        
        close(filedes[1]);

        fut.get();
        close(filedes[0]);
    }
}

bool checkPkgConfig(const string &name, shared_ptr<sleek::codeModel::SleekPackage> package)
{
    FILE *fp;
    int status;    

    fp = popen(("pkg-config --print-requires " + package->pkgConfigName).c_str(), "r");

    if (fp == NULL)
    {
        fprintf(stderr, "popen failure: %s\n", strerror(errno));
        throw runtime_error("popen failure");
    }

    sleek::util::ifdstream in(fileno(fp));

    for (string dependency; in >> dependency;)
    {
        package->dependencies.push_back(dependency);
    }

    status = pclose(fp);
    if (status == -1)
    {
        fprintf(stderr, "pclose failure: %s\n", strerror(errno));
        throw runtime_error("pclose failure");
    }

    return status == 0;
}