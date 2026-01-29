# TenEngine root (002-Object worktree)
cmake_minimum_required(VERSION 3.14)
project(TenEngine LANGUAGES CXX)

enable_testing()
add_subdirectory(modules/object)
# Tests are added by modules/object/tests/CMakeLists.txt when building from here
