#include "texture_reference.hpp"
#include "../ecsql/hook_system.hpp"
#include "../ecsql/sql_hook_row.hpp"

using namespace ecsql;

TextureFlyweight::autorelease_value TextureReference::get() {
    return texture_flyweight.get_autorelease(std::string(path));
}

void TextureReference::register_component(ecsql::Ecsql& world) {
    world.register_component(TextureReferenceComponent);
    world.register_on_insert_system({
        "TextureReference",
        [](SQLHookRow&, SQLHookRow& new_row) {
            texture_flyweight.get(new_row.get<const char *>(TextureReferenceComponent.first_field_index()));
        }
    });
    world.register_on_update_system({
        "TextureReference",
        [](SQLHookRow& old_row, SQLHookRow& new_row) {
            texture_flyweight.release(old_row.get<const char *>(TextureReferenceComponent.first_field_index()));
            texture_flyweight.get(new_row.get<const char *>(TextureReferenceComponent.first_field_index()));
        }
    });
    world.register_on_delete_system({
        "TextureReference",
        [](SQLHookRow& old_row, SQLHookRow&) {
            texture_flyweight.release(old_row.get<const char *>(TextureReferenceComponent.first_field_index()));
        }
    });
}
