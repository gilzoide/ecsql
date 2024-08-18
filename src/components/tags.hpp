#pragma once

#include "../ecsql/component.hpp"
#include "../ecsql/ecsql.hpp"

inline ecsql::Tag RotateOnHover { "RotateOnHover" };

void register_tags(ecsql::Ecsql& world);
