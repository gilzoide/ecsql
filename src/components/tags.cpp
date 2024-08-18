#include "tags.hpp"

void register_tags(ecsql::Ecsql& world) {
    world.register_component(RotateOnHover);
}
