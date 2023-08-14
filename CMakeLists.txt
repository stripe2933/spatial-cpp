cmake_minimum_required(VERSION 3.26)
project(spatial)

add_library(spatial src/spatial/utils/thrower.cpp)

target_compile_features(spatial PUBLIC cxx_std_20)
target_include_directories(spatial PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_directories(spatial PUBLIC ${CMAKE_SOURCE_DIR}/src)

if (BUILD_TESTING)
    add_subdirectory(test)
endif()