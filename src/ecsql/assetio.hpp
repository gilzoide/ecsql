#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include <physfs.h>
#include <sol/sol.hpp>

namespace ecsql {

void assetio_initialize(const char *argv0, const char *organization, const char *app_name, const char *archive_ext = "zip", bool archives_first = true);
void assetio_terminate();

std::vector<uint8_t> read_asset_bytes(const char *filename, int buffer_size = 1024);
std::string read_asset_text(const char *filename, int buffer_size = 1024);

sol::load_result load_lua_script(sol::state_view L, const char *filename, int buffer_size = 1024, sol::load_mode mode = sol::load_mode::any);
sol::load_result safe_load_lua_script(sol::state_view L, const char *filename, int buffer_size = 1024, sol::load_mode mode = sol::load_mode::any);
sol::protected_function_result do_lua_script(sol::state_view L, const char *filename, int buffer_size = 1024, sol::load_mode mode = sol::load_mode::any);
sol::protected_function_result safe_do_lua_script(sol::state_view L, const char *filename, int buffer_size = 1024, sol::load_mode mode = sol::load_mode::any);

struct PHYSFS_FileDeleter {
	void operator()(PHYSFS_File *file) {
		PHYSFS_close(file);
	}
};

template<typename Buffer>
Buffer read_asset_data(const char *filename, int buffer_size = 1024) {
	Buffer buffer;
	std::unique_ptr<PHYSFS_File, PHYSFS_FileDeleter> file(PHYSFS_openRead(filename));
	if (file) {
		while (true) {
			PHYSFS_sint64 current_size = buffer.size();
			buffer.resize(current_size + buffer_size);
			PHYSFS_sint64 read_size = PHYSFS_readBytes(file.get(), buffer.data() + current_size, buffer_size);
			if (read_size < buffer_size) {
				buffer.resize(current_size + read_size);
				break;
			}
		}
	}
	return buffer;
}

template<typename Fn>
void foreach_file(const char *dir, Fn&& f) {
	auto f_listed_files = std::make_pair(std::move(f), std::unordered_set<std::filesystem::path>());
	PHYSFS_enumerate(dir, [](void *data, const char *origdir, const char *fname) -> PHYSFS_EnumerateCallbackResult {
		auto&& [f, listed_files] = *((decltype(f_listed_files) *) data);
		std::filesystem::path path = std::filesystem::path(origdir) / fname;
		if (listed_files.find(path) == listed_files.end()) {
			f(path);
			listed_files.emplace(path);
			return PHYSFS_ENUM_OK;
		}
		else {
			return PHYSFS_ENUM_STOP;
		}
	}, (void *) &f_listed_files);
}

}
