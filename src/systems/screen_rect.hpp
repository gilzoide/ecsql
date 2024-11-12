#pragma once

#include "../ecsql/component.hpp"
#include "../ecsql/ecsql.hpp"

inline ecsql::Tag ScreenRectTag { "ScreenRect" };

void register_update_screen_rect(ecsql::Ecsql& world);
