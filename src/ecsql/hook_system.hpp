#pragma once

#include <functional>
#include <string>
#include <string_view>

#include "component.hpp"
#include "static_linked_list.hpp"

struct sqlite3;

namespace ecsql {

class SQLHookRow;

enum class HookType {
	OnInsert,
	OnUpdate,
	OnDelete,
};

class HookSystem {
public:
	template<typename Fn>
	HookSystem(HookType hook_type, std::string_view component_name, Fn&& implementation)
		: component_name(component_name)
		, implementation([=](SQLHookRow& old_row, SQLHookRow& new_row) { implementation(old_row, new_row); })
		, hook_type(hook_type)
	{
		STATIC_LINKED_LIST_INSERT();
	}

	template<typename Fn>
	HookSystem(HookType hook_type, const Component& component, Fn&& implementation)
		: HookSystem(hook_type, component.get_name(), implementation)
	{
	}

	void operator()(SQLHookRow& old_row, SQLHookRow& new_row) const;

	std::string component_name;
	std::function<void(SQLHookRow&, SQLHookRow&)> implementation;
	HookType hook_type;

	STATIC_LINKED_LIST_DEFINE(HookSystem);
};

}
