#include "texture_reference.hpp"
#include "../ecsql/hook_system.hpp"
#include "../ecsql/sql_hook_row.hpp"

using namespace ecsql;

TextureFlyweight::autorelease_value TextureReference::get() {
    return texture_flyweight.get_autorelease(path);
}

void TextureReference::register_component(ecsql::Ecsql& world) {
    world.register_component(TextureReferenceComponent);
    world.register_on_insert_system({
        "TextureReference",
        [](const SQLHookRow& row) {
            texture_flyweight.get(row.get<const char *>(2));
        }
    });
    world.register_on_delete_system({
        "TextureReference",
        [](const SQLHookRow& row) {
            texture_flyweight.release(row.get<const char *>(2));
        }
    });
}

Component TextureReferenceComponent = Component::from_type<TextureReference>();
