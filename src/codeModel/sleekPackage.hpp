#pragma once
#include <string>
#include <vector>

namespace sleek
{
namespace codeModel
{

// SleekPackage
// The data for objects of this class can be populated from pkg-config if available.
// There are two special packages, system and this.
// System package as no dependencies and is available by default.
// This package is this project and all dependencies should be contained in the project itself.
class SleekPackage
{
public:
    std::string name;
    std::string pkgConfigName;
    std::string version;
    std::string description;
    std::string homePage;
    std::string source;
    std::vector<std::string> dependencies;
    std::vector<std::string> cflags;
    std::vector<std::string> libs;
    std::vector<std::string> libsPrivate;
    std::vector<std::string> headersProvided;
    bool isThirdParty;
    bool system;
};

}
}