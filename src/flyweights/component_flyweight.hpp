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
	ComponentFlyweight(std::string_view component_name = reflect::type_name<T>(), std::string_view key_name = "path")
		: component(component_name, { std::string(key_name) })
	{
	}

	template<typename Creator>
	ComponentFlyweight(Creator&& creator, std::string_view component_name = reflect::type_name<T>(), std::string_view key_name = "path")
		: flyweight(creator)
		, component(component_name, { std::string(key_name) })
	{
	}

	template<typename Creator, typename Deleter>
	ComponentFlyweight(Creator&& creator, Deleter&& deleter, std::string_view component_name = reflect::type_name<T>(), std::string_view key_name = "path")
		: flyweight(creator, deleter)
		, component(component_name, { std::string(key_name) })
	{
	}

	template<typename U>
	flyweight::flyweight_refcounted<Key, T>::autorelease_value_type get(U&& key) {
		return flyweight.get_autorelease(Key(std::forward<U>(key)));
	}

	flyweight::flyweight_refcounted<Key, T> flyweight;
	RawComponent component;
	
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
