cmake_minimum_required(VERSION 3.5)

project(game2d LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DGAME_DEBUG")

########################
# External dependencies
########################
# sfml
set(SFML_DIR "${CMAKE_SOURCE_DIR}/deps/SFML-2.5.1/lib/cmake/SFML")
find_package(SFML 2.5 COMPONENTS graphics audio REQUIRED)

# box2d
include_directories("${CMAKE_SOURCE_DIR}/deps/")
set (BOX2D_LIBRARY "${CMAKE_SOURCE_DIR}/deps/box2d-2.3.1/libBox2D.a")

# opengl
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
if (OPENGL_FOUND)
  include_directories(${OPENGL_INCLUDE_DIRS})
endif()

# imgui
include_directories("deps/imgui-1.76/")
file(GLOB imgui_src "deps/imgui-1.76/imgui.cpp"
                    "deps/imgui-1.76/imgui_widgets.cpp"
                    "deps/imgui-1.76/imgui_draw.cpp"
                    "deps/imgui-1.76/imgui-SFML.cpp"
                    "deps/imgui-1.76/imgui_demo.cpp")

# Internal
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/)
include_directories("src/")
file(GLOB src_files "src/*.cpp")
file(GLOB inc_files "src/*.hpp")

# Linking
add_executable(game2d main.cpp ${src_files} ${inc_files} ${imgui_src})
target_precompile_headers(game2d PRIVATE "pch.h")
target_link_libraries(game2d
                 ${BOX2D_LIBRARY}
                 sfml-graphics
                 sfml-audio
                 ${SFML_DEPENDENCIES}
                 ${OPENGL_LIBRARIES})
