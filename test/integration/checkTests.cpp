#include <gtest/gtest.h>

#include "check.hpp"

sleek::codeModel::ProjectOptions projectOptions {
    name: "checkTests",
    globalInstall: false,
    sourceDir: ".",
    buildDir: "build",
    includeDir: "include"
};

// Check returns normally.
TEST(checkTests, CheckCommandDoesNotCrash) {
    auto project = sleek::codeModel::Project::initializeProject(projectOptions);
    sleek::command::check(project);
}