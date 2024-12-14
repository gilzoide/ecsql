#pragma once

#include <functional>
#include <string>
#include <string_view>

#include "component.hpp"
#include "static_linked_list.hpp"

struct sqlite3;

namespace ecsql {

class SQLBaseRow;

enum class HookType {
	OnInsert,
	OnUpdate,
	OnDelete,
};

class HookSystem {
public:
	template<typename Fn>
	HookSystem(std::string_view component_name, Fn&& implementation)
		: component_name(component_name)
		, implementation([=](HookType hook, SQLBaseRow& old_row, SQLBaseRow& new_row) { implementation(hook, old_row, new_row); })
	{
		STATIC_LINKED_LIST_INSERT();
	}

	template<typename Fn>
	HookSystem(const Component& component, Fn&& implementation)
		: HookSystem(component.get_name(), implementation)
	{
	}

	void operator()(HookType hook, SQLBaseRow& old_row, SQLBaseRow& new_row) const;

	std::string component_name;
	std::function<void(HookType, SQLBaseRow&, SQLBaseRow&)> implementation;

	STATIC_LINKED_LIST_DEFINE(HookSystem);
};

}
