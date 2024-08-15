#pragma once

#include <string>

#include <raylib.h>

#include <flyweight.hpp>

using TextureFlyweight = flyweight::flyweight_refcounted<Texture2D, std::string>;
extern TextureFlyweight texture_flyweight;
