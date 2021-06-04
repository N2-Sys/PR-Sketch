#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "./util.h"

FILE* fopen_with_dirs(const char* filename, const char* mode) {
	uint32_t total_len = strlen(filename);

	char tmp_dirname[total_len];
	memset(tmp_dirname, '\0', total_len);
	uint32_t cur_pos = 0;
	const char* cur_pointer = strchr(filename, '/');
	int status = 0;
	while (true) {
		if (cur_pointer == NULL) {
			break;
		}

		cur_pos = uint32_t(cur_pointer - filename);
		if (cur_pos == 0) {
			continue;
		}
		strncpy(tmp_dirname, filename, cur_pos);
		status = access(tmp_dirname, 0);
		if (status == -1) { // tmp_dirname does not exist
			status = mkdir(tmp_dirname, 0755);
			if (status == -1) {
				LOG_ERR("Cannot mkdir %s: %s\n", tmp_dirname, strerror(errno));
			}
		}

		cur_pointer = strchr(cur_pointer+1, '/');
	}

	FILE* fd = NULL;
	fd = fopen(filename, mode);
	afs_assert(fd!=NULL, "Cannot create the file %s: %s\n", filename, strerror(errno));
	return fd;
}
