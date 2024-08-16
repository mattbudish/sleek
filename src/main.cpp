//
//                   _____  __             __
//                  / ___/ / /___   ___   / /__
//                  \__ \ / // _ \ / _ \ / //_/
//                 ___/ // //  __//  __// ,<
//                /____//_/ \___/ \___//_/|_|
//
//
// Sleek Build System
//
// Build C/C++ projects without build files.
//
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// Copyright 2023 Matt Budish <mtbudish@gmail.com>

#include <argparse/argparse.hpp>
#include <filesystem>

#include "codeModel/project.hpp"
#include "command/build.hpp"
#include "command/check.hpp"
#include "command/fetch.hpp"
#include "command/install.hpp"
#include "command/pack.hpp"
#include "command/publish.hpp"
#include "command/test.hpp"
#include "command/uninstall.hpp"

using namespace std;
namespace fs = std::filesystem;

// Sleek
int main(int argc, char *argv[])
{
    sleek::codeModel::ProjectOptions projectOptions;
    argparse::ArgumentParser program("sleek", "0.1.0");

    program.add_description("Sleek Build System -- build projects without build files");

    program.add_argument("-n", "--name")
        .help("sets the name of the project, default is the name of the current directory")
        .default_value(string(fs::current_path().filename()))
        .metavar("NAME");

    program.add_argument("-b", "--build-dir")
        .help("sets the directory in which artifacts will be built")
        .default_value(string("build"))
        .metavar("DIR");

    program.add_argument("-s", "--source-dir")
        .help("sets the source directory")
        .default_value(string("src"))
        .metavar("DIR");

    program.add_argument("-l", "--lib-dir")
        .help("sets the library directory (if there is one)")
        .default_value(string("lib"))
        .metavar("DIR");

    program.add_argument("-i", "--include-dir")
        .help("sets the include directory (if there is one)")
        .default_value(string("include"))
        .metavar("DIR");

    program.add_argument("-t", "--target")
        .help("only applies to a specific target, default is apply to all targets")
        .default_value(string(""))
        .metavar("TARGET");

    // sleek build subparser
    argparse::ArgumentParser build_command("build");
    build_command.add_description("Build code in current directory");

    // sleek fetch subparser
    argparse::ArgumentParser fetch_command("fetch");
    fetch_command.add_description("Fetch dependencies without building project");

    // sleek install subparser
    argparse::ArgumentParser install_command("install");
    install_command.add_description("Install the dependencies in the local sleek_packages folder");
    install_command.add_argument("-g", "--global")
        .help("installs the current package and any runtime dependencies globally")
        .default_value(false)
        .implicit_value(true);

    install_command.add_argument("packages")
        .help("installs package[s] specified")
        .default_value(std::vector<std::string>())
        .remaining();

    // sleek uninstall subparser
    argparse::ArgumentParser uninstall_command("uninstall");
    uninstall_command.add_description("Uninstall artifacts of this build (if they have been installed)");

    // sleek check subparser
    argparse::ArgumentParser check_command("check");
    check_command.add_description("Check current directory for consistency with Sleek");

    // sleek test subparser
    argparse::ArgumentParser test_command("test");
    test_command.add_description("Build and run unit tests");

    // sleek test subparser
    argparse::ArgumentParser pack_command("pack");
    pack_command.add_description("Create package based on current project");

    // sleek publish subparser
    argparse::ArgumentParser publish_command("publish");
    publish_command.add_description("Upload package to registry");

    program.add_subparser(build_command);
    program.add_subparser(fetch_command);
    program.add_subparser(install_command);
    program.add_subparser(uninstall_command);
    program.add_subparser(check_command);
    program.add_subparser(test_command);
    program.add_subparser(pack_command);
    program.add_subparser(publish_command);

    try
    {
        program.parse_args(argc, argv);
    }
    catch(const std::runtime_error& err) 
    {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    projectOptions.name = program.get<string>("-n");
    projectOptions.buildDir = program.get<string>("-b");
    projectOptions.sourceDir = program.get<string>("-s");
    projectOptions.libDir = program.get<string>("-l");
    projectOptions.includeDir = program.get<string>("-i");
    projectOptions.selectedTarget = program.get<string>("-t");

    if (program.is_subcommand_used("build"))
    {
        projectOptions.command = sleek::command::build;
    }
    else if (program.is_subcommand_used("fetch"))
    {
        projectOptions.command = sleek::command::fetch;
    }
    else if (program.is_subcommand_used("install"))
    {
        projectOptions.globalInstall = install_command.get<bool>("-g");
        projectOptions.packageNames = install_command.get<std::vector<std::string>>("packages");
        projectOptions.command = sleek::command::install;
    }
    else if (program.is_subcommand_used("uninstall"))
    {
        projectOptions.command = sleek::command::uninstall;
    }
    else if (program.is_subcommand_used("check"))
    {
        projectOptions.command = sleek::command::check;
    }
    else if (program.is_subcommand_used("test"))
    {
        projectOptions.command = sleek::command::test;
    }
    else if (program.is_subcommand_used("pack"))
    {
        projectOptions.command = sleek::command::pack;
    }
    else if (program.is_subcommand_used("publish"))
    {
        projectOptions.command = sleek::command::publish;
    }
    else
    {
        std::cerr << program;
        return 1;
    }

    auto project = sleek::codeModel::Project::initializeProject(projectOptions);

    projectOptions.command(project);

    return 0;
}