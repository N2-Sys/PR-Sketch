#include "ancillary.h"

const char* path_join(const char* dirname, const char* filename) {
	if (dirname == NULL) {
		LOG_ERR(PATH_LOG_PREFIX "path_join: dirname is null!\n");
	}

	if (filename == NULL) {
		LOG_ERR(PATH_LOG_PREFIX "path_join: filename is null!\n");
	}

	int dirname_len = strlen(dirname);
	int filename_len = strlen(filename);

	int sum_len = dirname_len + filename_len + 1;
	if ( sum_len > MAX_PATH_LENGTH) {
		LOG_ERR(PATH_LOG_PREFIX "path_join: path length is larger than %d!\n", sum_len);
	}

	char* path = (char*) malloc(MAX_PATH_LENGTH + 1);
	strcpy(path, dirname);
	strcat(path, "/");
	strcat(path, filename);

	return path;
}
