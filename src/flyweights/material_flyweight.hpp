#pragma once

#include <span>

#include <raylib.h>

#include "component_flyweight.hpp"

extern ComponentFlyweight<std::span<Material>> MaterialSetFlyweight;
