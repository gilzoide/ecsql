#include <raylib.h>
#include <cstdlib>

#include "assetio.hpp"

namespace assetio {

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

void initialize(const char *argv0, const char *organization, const char *app_name) {
	PHYSFS_init(argv0);
	PHYSFS_setSaneConfig(organization, app_name, "zip", 0, 1);

	SetLoadFileDataCallback(LoadFileDataCallback);
	SetSaveFileDataCallback(SaveFileDataCallback);
	SetLoadFileTextCallback(LoadFileTextCallback);
	SetSaveFileTextCallback(SaveFileTextCallback);
}

void terminate() {
	PHYSFS_deinit();
}

}