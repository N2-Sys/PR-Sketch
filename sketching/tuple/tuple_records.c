#include <stdio.h>
#include <errno.h>

#include "./tuple_records.h"

TupleRecords::TupleRecords(string filepath){
	this->filepath = filepath;
}

vector<tuple_t> TupleRecords::read_all(){
	if (IS_CLICK) {
		return this->read_all_clicks();
	}

	uint32_t tuple_size = sizeof(tuple_t);
	vector<tuple_t> tuple_vector;

	FILE* record_fd = fopen(filepath.c_str(), "rb");
	if (record_fd == NULL) {
		LOG_ERR("Cannot open %s\n", filepath.c_str());
	}

	while (true) {
		tuple_t t;
		int read_cnt = fread(&t, tuple_size, 1, record_fd);
		if (read_cnt != 1) {
			if (ferror(record_fd)) {
				LOG_ERR("read record file error: %s\n",strerror(errno));
			}
			else if (feof(record_fd)) {
				//LOG_MSG("read record file end\n");
				break;
			}
			else {
				LOG_ERR("read record file error: unknown error\n");
			}
		}
		tuple_vector.push_back(t);
	}
	fclose(record_fd);

	return tuple_vector;
}

vector<tuple_t> TupleRecords::read_all_clicks() {
	vector<tuple_t> tuple_vector;

	FILE* record_fd = fopen(filepath.c_str(), "r");
	if (record_fd == NULL) {
		LOG_ERR("Cannot open %s\n", filepath.c_str());
	}

	char* line = NULL;
	size_t len = 0;
	ssize_t read = 0;
	while (true) {
		read = getline(&line, &len, record_fd);
		if (read == -1) {
			break;
		}

		char* startpos = line;
		while ((startpos-line) < len) {
			char tmpstr[100];
			memset(tmpstr, '\n', 100);

			char* pos = strchr(startpos, ' ');
			if (pos == NULL) {
				strcpy(tmpstr, startpos);
				startpos = line + len; // end of loop
			}
			else {
				strncpy(tmpstr, startpos, pos-startpos);
				startpos = pos + 1; // might be end of loop
			}

			uint32_t tmpvalue = uint32_t(atoi(tmpstr));
			tuple_t t;
			memset(&t, 0, sizeof(tuple_t));
			t.key.src_ip = tmpvalue;
			t.key.dst_ip = tmpvalue;
			t.size = 64;
			tuple_vector.push_back(t);
		}
	}

	if (line != NULL) {
		free(line);
	}
	fclose(record_fd);
	return tuple_vector;
}

vector<tuple_key_t> TupleRecords::read_all_tuplekey() {
	vector<tuple_t> tuple_vector = this->read_all();
	vector<tuple_key_t> tuplekey_vector;
	for (uint32_t i = 0; i < tuple_vector.size(); i++) {
		tuple_t t = tuple_vector[i];
		tuple_key_t tmp_tuplekey;
		tmp_tuplekey.src_ip = t.key.src_ip;
		tmp_tuplekey.dst_ip = t.key.dst_ip;
		tuplekey_vector.push_back(tmp_tuplekey);
	}
	return tuplekey_vector;
}

uint32_t TupleRecords::get_bytesum() {
	vector<tuple_t> tuple_vector = this->read_all();
	uint32_t bytesum = 0;
	for (uint32_t i = 0; i < tuple_vector.size(); i++) {
		bytesum += tuple_vector[i].size;
	}
	return bytesum;
}
