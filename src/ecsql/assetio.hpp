#pragma once

#include <filesystem>
#include <memory>
#include <string>
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

struct PHYSFS_FileListDeleter {
	void operator()(char **file_list) {
		PHYSFS_freeList(file_list);
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
void foreach_file(std::filesystem::path dir, Fn&& f, bool recursive = false) {
	std::unique_ptr<char *, PHYSFS_FileListDeleter> list(PHYSFS_enumerateFiles(dir.c_str()));
	if (list) {
		PHYSFS_Stat stat;
		for (char **entry = list.get(); *entry != nullptr; entry++) {
			std::filesystem::path full_path = dir / *entry;
			PHYSFS_stat(full_path.c_str(), &stat);
			if (stat.filetype == PHYSFS_FILETYPE_REGULAR) {
				f(full_path);
			}
			else if (recursive) {
				foreach_file(full_path, f, true);
			}
		}
	}
}

}
