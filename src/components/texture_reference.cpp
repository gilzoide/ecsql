#include "texture_reference.hpp"
#include "../ecsql/hook_system.hpp"
#include "../ecsql/sql_hook_row.hpp"

using namespace ecsql;

TextureFlyweight::autorelease_value TextureReference::get() {
    return texture_flyweight.get_autorelease(std::string(path));
}

HookSystem OnInsertTextureReference {
	HookSystem::HookType::Insert,
	TextureReferenceComponent,
	[](SQLHookRow&, SQLHookRow& new_row) {
		texture_flyweight.get(new_row.get<const char *>(TextureReferenceComponent.first_field_index()));
	}
};

HookSystem OnUpdateTextureReference {
	HookSystem::HookType::Update,
	TextureReferenceComponent,
	[](SQLHookRow& old_row, SQLHookRow& new_row) {
		texture_flyweight.release(old_row.get<const char *>(TextureReferenceComponent.first_field_index()));
		texture_flyweight.get(new_row.get<const char *>(TextureReferenceComponent.first_field_index()));
	}
};

HookSystem OnDeleteTextureReference {
	HookSystem::HookType::Delete,
	TextureReferenceComponent,
	[](SQLHookRow& old_row, SQLHookRow&) {
		texture_flyweight.release(old_row.get<const char *>(TextureReferenceComponent.first_field_index()));
	}
};
