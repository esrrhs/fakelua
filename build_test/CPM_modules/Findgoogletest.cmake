include("N:/project/fakelua/build_test/cmake/CPM_0.38.7.cmake")
CPMAddPackage("NAME;googletest;GITHUB_REPOSITORY;google/googletest;GIT_TAG;v1.17.0;VERSION;1.17.0;OPTIONS;INSTALL_GTEST OFF;gtest_force_shared_crt ON")
set(googletest_FOUND TRUE)