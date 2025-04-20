#include <filesystem>

#include <raylib.h>

#include "assetio.hpp"

namespace ecsql {

// Raylib functions
static unsigned char *LoadFileDataCallback(const char *fileName, int *dataSize) {
	std::unique_ptr<PHYSFS_File, PHYSFS_FileDeleter> file(PHYSFS_openRead(fileName));
	if (file) {
		PHYSFS_sint64 filesize = PHYSFS_fileLength(file.get());
		if (unsigned char *buffer = (unsigned char *) RL_MALLOC(filesize)) {
			*dataSize = PHYSFS_readBytes(file.get(), buffer, filesize);
			return buffer;
		}
	}
	return nullptr;
}

static bool SaveFileDataCallback(const char *fileName, void *data, int dataSize) {
	std::unique_ptr<PHYSFS_File, PHYSFS_FileDeleter> file(PHYSFS_openWrite(fileName));
	if (file) {
		return PHYSFS_writeBytes(file.get(), data, dataSize) == dataSize;
	}
	return false;
}

static char *LoadFileTextCallback(const char *fileName) {
	std::unique_ptr<PHYSFS_File, PHYSFS_FileDeleter> file(PHYSFS_openRead(fileName));
	if (file) {
		PHYSFS_sint64 filesize = PHYSFS_fileLength(file.get());
		if (char *buffer = (char *) RL_MALLOC(filesize + 1)) {
			PHYSFS_sint64 read_bytes = PHYSFS_readBytes(file.get(), buffer, filesize);
			buffer[read_bytes] = '\0';
			return buffer;
		}
	}
	return nullptr;
}

static bool SaveFileTextCallback(const char *fileName, char *text) {
	std::unique_ptr<PHYSFS_File, PHYSFS_FileDeleter> file(PHYSFS_openWrite(fileName));
	if (file) {
		for (const char *c = text; *c; c++) {
			PHYSFS_sint64 written_bytes = PHYSFS_writeBytes(file.get(), c, 1);
			if (written_bytes <= 0) {
				return false;
			}
		}
		return true;
	}
	return false;
}

// Lua reading
struct PHYSFS_LuaReader {
	PHYSFS_LuaReader(const char *filename, int buffer_size = 1024)
		: file(PHYSFS_openRead(filename))
		, buffer(buffer_size)
		, buffer_size(buffer_size)
	{
	}

	const char *read(size_t *size) {
		if (!file) {
			return nullptr;
		}
		PHYSFS_sint64 read_size = PHYSFS_readBytes(file.get(), buffer.data(), buffer_size);
		if (read_size < 0) {
			return nullptr;
		}
		*size = read_size;
		return buffer.data();
	}

	operator bool() const {
		return file.get();
	}

	std::unique_ptr<PHYSFS_File, PHYSFS_FileDeleter> file;
	std::vector<char> buffer;
	int buffer_size;
};

static const char *lua_reader(lua_State *L, void *data, size_t *size) {
	return ((PHYSFS_LuaReader *) data)->read(size);
}

// assetio API
void assetio_initialize(const char *argv0, const char *organization, const char *app_name, const char *archive_ext, bool archives_first) {
	PHYSFS_init(argv0);
	PHYSFS_setSaneConfig(organization, app_name, archive_ext, 0, archives_first);

#if defined(DEBUG) && !defined(NDEBUG) && !defined(__EMSCRIPTEN__)
	// In debug builds, mount the first "assets" folder found in parent directories
	// This enables us to rerun the game after changing assets without having to rebuild
	for (std::filesystem::path dir = PHYSFS_getBaseDir(); dir.has_relative_path(); dir = dir.parent_path()) {
		std::filesystem::path assets_path = dir / "assets";
		if (std::filesystem::exists(assets_path)) {
			if (PHYSFS_mount( assets_path.c_str(), nullptr, 0)) {
				std::cout << "DEBUG: Mounted assets folder: " << assets_path << std::endl;
			}
			else {
				std::cerr << "DEBUG: Error mounting 'assets': " << PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()) << std::endl;
			}
			break;
		}
	}
#endif

	SetLoadFileDataCallback(LoadFileDataCallback);
	SetSaveFileDataCallback(SaveFileDataCallback);
	SetLoadFileTextCallback(LoadFileTextCallback);
	SetSaveFileTextCallback(SaveFileTextCallback);
}

void assetio_terminate() {
	SetLoadFileDataCallback(nullptr);
	SetSaveFileDataCallback(nullptr);
	SetLoadFileTextCallback(nullptr);
	SetSaveFileTextCallback(nullptr);

	PHYSFS_deinit();
}

std::vector<uint8_t> read_asset_bytes(const char *filename, int buffer_size) {
	return read_asset_data<std::vector<uint8_t>>(filename, buffer_size);
}

std::string read_asset_text(const char *filename, int buffer_size) {
	return read_asset_data<std::string>(filename, buffer_size);
}

sol::load_result safe_load_lua_script(sol::state_view L, const char *filename, int buffer_size, sol::load_mode mode) {
	PHYSFS_LuaReader reader(filename, buffer_size);
	if (!reader) {
		lua_pushfstring(L, "Cannot open file %s", filename);
		return sol::load_result(L, -1, 0, 1, sol::load_status::file);
	}
	return L.load(lua_reader, &reader, filename, mode);
}

sol::load_result load_lua_script(sol::state_view L, const char *filename, int buffer_size, sol::load_mode mode) {
	auto result = safe_load_lua_script(L, filename, buffer_size, mode);
	if (result.valid()) {
		return result;
	}
	else {
		throw result.get<sol::error>();
	}
}

sol::protected_function_result safe_do_lua_script(sol::state_view L, const char *filename, int buffer_size, sol::load_mode mode) {
	PHYSFS_LuaReader reader(filename, buffer_size);
	if (!reader) {
		lua_pushfstring(L, "Cannot open file %s", filename);
		return sol::protected_function_result(L, -1, 0, 1, sol::call_status::file);
	}
	return L.do_reader(lua_reader, &reader, filename, mode);
}

sol::protected_function_result do_lua_script(sol::state_view L, const char *filename, int buffer_size, sol::load_mode mode) {
	auto result = safe_do_lua_script(L, filename, buffer_size, mode);
	if (result.valid()) {
		return result;
	}
	else {
		throw result.get<sol::error>();
	}
}

}
