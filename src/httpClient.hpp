// httpClient.hpp
#pragma once

#include <ostream>

namespace sleek
{

namespace net
{

long httpClient(const std::string &url, std::ostream &http_data);
    
} // namespace net

}