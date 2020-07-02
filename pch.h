#pragma once

#include "mytypes.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <queue>
#include <map>
#include <unordered_map>
#include <memory>
#include <cmath>    // abs
#include <cstddef>  // offsetof, NULL
#include <cstring>  // memcpy
#include <functional>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include <SFML/OpenGL.hpp>
#include <SFML/Config.hpp>

#include "Box2D/Box2D.h"

#include "imgui-SFML.h"
#include "imgui.h"

//#include <glm/glm.hpp>

#define BUILD_DEBUG
#ifdef BUILD_DEBUG
    #define DEBUG_GUI(x) g_gui_callbacks.push_back(x);
#else
    #define DEBUG_GUI(x)
#endif

extern std::vector<std::function<void(void)>> g_gui_callbacks;
