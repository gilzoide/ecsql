#include <raylib.h>

#include "assetio.hpp"

namespace ecsql {

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

void assetio_initialize(const char *argv0, const char *organization, const char *app_name, const char *archive_ext, bool archives_first) {
	PHYSFS_init(argv0);
	PHYSFS_setSaneConfig(organization, app_name, archive_ext, 0, archives_first);

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

}
