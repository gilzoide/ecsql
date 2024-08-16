#pragma once

#include <string>

#include "../ecsql/component.hpp"
#include "../ecsql/ecsql.hpp"
#include "../flyweights/texture_flyweight.hpp"

struct TextureReference {
    const char *path;

    TextureFlyweight::autorelease_value get();

    static void register_component(ecsql::Ecsql& world);
};

extern ecsql::Component TextureReferenceComponent;
