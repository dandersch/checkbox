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
find_package(SFML 2.5 COMPONENTS graphics audio REQUIRED)

# box2d
#find_package(Box2D REQUIRED)
#if (BOX2D_FOUND)
#  include_directories(${BOX2D_INCLUDE_DIR})
#endif()
#include_directories(/usr/include/box2d/)

# glm
#find_package(glm REQUIRED)
#if (GLM_FOUND)
#    include_directories(${GLM_INCLUDE_DIRS})
#endif()

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
		            "deps/imgui-1.76/imgui_demo.cpp"
                    )

# Internal
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/)
include_directories("src/")
file(GLOB src_files "src/*.cpp")
file(GLOB inc_files "src/*.hpp")

#add_library(MY_SFML SHARED
            #"${CMAKE_SOURCE_DIR}/deps/libsfml-system.so.2.5.1"
            #"${CMAKE_SOURCE_DIR}/deps/libsfml-window.so.2.5.1"
            #"${CMAKE_SOURCE_DIR}/deps/libsfml-graphics.so.2.5.1"
            #"${CMAKE_SOURCE_DIR}/deps/libsfml-audio.so.2.5.1"
            #)

# Linking
add_executable(game2d main.cpp ${src_files} ${inc_files} ${imgui_src})
target_precompile_headers(game2d PRIVATE "pch.h")
target_link_libraries(game2d
                 "${CMAKE_SOURCE_DIR}/deps/box2d-2.3.1/libBox2D.a"
                 sfml-graphics
			     sfml-audio
			     #${BOX2D_LIBRARY}
			     #box2d
			     #glm
			     ${OPENGL_LIBRARIES}
			     )
