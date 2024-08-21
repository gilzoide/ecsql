#pragma once

#include <string>

#include "../ecsql/component.hpp"
#include "../ecsql/ecsql.hpp"
#include "../flyweights/texture_flyweight.hpp"

struct TextureReference {
    std::string_view path;

    TextureFlyweight::autorelease_value get();
};

inline ecsql::Component<TextureReference> TextureReferenceComponent;
