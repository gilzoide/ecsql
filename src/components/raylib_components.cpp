#include "raylib_components.hpp"
#include "../ecsql/component.hpp"

void register_raylib_components(ecsql::Ecsql& world) {
    world.register_component(ColorComponent);
    world.register_component(RectangleComponent);
}
