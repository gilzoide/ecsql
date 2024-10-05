#pragma once

#include <string>
#include <string_view>

#include <reflect>
#include <flyweight.hpp>

#include "../ecsql/component.hpp"
#include "../ecsql/hook_system.hpp"
#include "../ecsql/sql_hook_row.hpp"

using namespace ecsql;


template<typename T, typename Key = std::string>
struct ComponentFlyweight {
	ComponentFlyweight() : flyweight() {}

	template<typename Creator>
	ComponentFlyweight(Creator&& creator) : flyweight(creator) {}

	template<typename Creator, typename Deleter>
	ComponentFlyweight(Creator&& creator, Deleter&& deleter) : flyweight(creator, deleter) {}

	template<typename U>
	flyweight::flyweight_refcounted<T, Key>::autorelease_value get(const U& key) {
		return flyweight.get_autorelease(Key(key));
	}

	flyweight::flyweight_refcounted<T, Key> flyweight;
	
	RawComponent component { reflect::type_name<T>(), { "path" } };
	
	HookSystem on_insert {
		HookType::OnInsert,
		component,
		[this](SQLHookRow&, SQLHookRow& new_row) {
			flyweight.get(new_row.get<const char *>(component.first_field_index()));
		}
	};

	HookSystem on_update {
		HookType::OnUpdate,
		component,
		[this](SQLHookRow& old_row, SQLHookRow& new_row) {
			flyweight.release(old_row.get<const char *>(component.first_field_index()));
			flyweight.get(new_row.get<const char *>(component.first_field_index()));
		}
	};

	HookSystem on_delete {
		HookType::OnDelete,
		component,
		[this](SQLHookRow& old_row, SQLHookRow&) {
			flyweight.release(old_row.get<const char *>(component.first_field_index()));
		}
	};
};
