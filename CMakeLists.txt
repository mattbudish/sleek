##
##                   _____  __             __
##                  / ___/ / /___   ___   / /__
##                  \__ \ / // _ \ / _ \ / //_/
##                 ___/ // //  __//  __// ,<
##                /____//_/ \___/ \___//_/|_|
##
##
## Sleek Build System
##
## Build C/C++ projects without build files.
##
## Licensed under the MIT License <http://opensource.org/licenses/MIT>.
## Copyright 2023 Matt Budish <mtbudish@gmail.com>

cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

##
## PROJECT
## name and version
##
project(sleek 
    VERSION 0.1.0 
    DESCRIPTION "Build C/C++ projects without build files."
    LANGUAGES CXX
)

option(Sleek_BuildTests "Build unit and integration tests." OFF)

add_subdirectory(src)

if(Sleek_BuildTests)
    include(CTest)
    enable_testing()
    add_subdirectory(test)
endif()
