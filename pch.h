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

#include "box2d/box2d.h"

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

// conversion functions
constexpr f64 PIXELS_PER_METER = 32.0;
constexpr f64 PI = 3.14159265358979323846;

template<typename T>
constexpr T pixelsToMeters(const T& x) { return x / PIXELS_PER_METER; };

template<typename T>
constexpr T metersToPixels(const T& x) { return x * PIXELS_PER_METER; };

template<typename T>
constexpr T degToRad(const T& x) { return PI * x / 180.f; }

template<typename T>
constexpr T radToDeg(const T& x) { return 180.f * x / PI; };
