#include <filesystem>
#include <fstream>
#include <future>
#include <nlohmann/json.hpp>
#include <unistd.h>
#include <vector>

#include "extract.hpp"
#include "fdstream.hpp"
#include "httpClient.hpp"
#include "install.hpp"

using namespace std;
using json = nlohmann::json;
namespace fs = std::filesystem;


void installGlobally(shared_ptr<sleek::codeModel::Project> project);
void installPackage(const string &name);

void sleek::command::install(shared_ptr<codeModel::Project> project)
{
    if (project->options.globalInstall)
    {
        installGlobally(project);
    }
    else if (project->options.packageNames.size() >= 1)
    {
        for (const auto &package : project->options.packageNames)
        {
            installPackage(package);
        }
    }
    else
    {
        std::cout << "Not supported" << std::endl;
    }
}

void installGlobally(shared_ptr<sleek::codeModel::Project> project)
{
    string name = project->name;
    string from;
    string to;
    
    // Copy file from build directory to /usr/local/bin.
    from = project->options.buildDir + "/" + project->options.sourceDir + "/" + name;
    to = "/usr/local/bin/" + name;

    const auto copyOptions = fs::copy_options::update_existing;

    try
    {
        fs::copy(from, to, copyOptions);
        cout << from << " has been successfully installed to " << to << "." << endl;
    }
    catch(const exception& e)
    {
        cerr << "Failure to install " << name << ": " << e.what() << '\n';
    }
}

void installPackage(const string &name)
{
    fs::directory_entry pkgDef { "/var/lib/sleek/index/" + name + ".json" };

    if (!pkgDef.exists())
    {
        cout << name << " not found in registry." << endl;
        return;
    }

    ifstream inFile = pkgDef.path();
    json j;

    inFile >> j;

    string url = j["/package/stable/files/x86_64_linux/url"_json_pointer];

    fs::create_directory(".sleek");
    fs::current_path(".sleek");

    int filedes[2];

    if (-1 == pipe(filedes))
    {
        throw runtime_error("Failure to open pipe.");
    }

    future<void> fut = async([](int fd){
        sleek::archive::extract(fd, 1);
    }, filedes[0]);

    sleek::util::ofdstream out_stream { filedes[1] };

    cout << "Downloading " << name << " from " << url << endl;

    auto http_code = sleek::net::httpClient(url, out_stream);
    if (http_code == 200)
    {
        cout << "Succcessfully installed " << name << endl;
    }
    else
    {
        cout << "Failure to download " << name << " HTTP code: " << http_code << endl;
    }
    
    close(filedes[1]);
}