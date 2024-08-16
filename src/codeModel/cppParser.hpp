#pragma once
#include <functional>
#include <string>
#include <vector>

namespace sleek
{
namespace codeModel
{

enum class IncludeStyle { Angle, Quote };

void parseFile(
    const std::string &fileName, 
    std::function<void(const std::string &, IncludeStyle)>, 
    std::function<void(void)>);

//std::function<void(const std::string &)> headerFileCb;

//std::function<void(void)> mainFoundCb;

}
}