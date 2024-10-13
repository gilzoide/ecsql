#pragma once

#include <memory>
#include <string>
#include <vector>

#include <physfs.h>

namespace assetio {

void initialize(const char *argv0, const char *organization, const char *app_name);
void terminate();

std::vector<uint8_t> read_asset_bytes(const char *filename, int buffer_size = 1024);
std::string read_asset_text(const char *filename, int buffer_size = 1024);

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

}
