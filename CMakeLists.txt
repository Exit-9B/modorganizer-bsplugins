cmake_minimum_required(VERSION 3.22)

# TODO: clean include directives
set(MO2_CMAKE_DEPRECATED_UIBASE_INCLUDE ON)

project(bsplugins)
set(project_type plugin)
set(enable_warnings OFF)

add_subdirectory(src)
