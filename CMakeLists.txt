cmake_minimum_required(VERSION 3.5)

project(game2d LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

# External dependencies
find_package(SFML 2.5 COMPONENTS graphics audio REQUIRED)
find_package(glm REQUIRED)
if (GLM_FOUND)
    include_directories(${GLM_INCLUDE_DIRS})
endif()

# Internal
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB src_files "src/*.cpp")

# Linking
add_executable(game2d main.cpp)
target_link_libraries(game2d ${src_files}
                             sfml-graphics
			     sfml-audio
			     glm)
