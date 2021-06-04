/*********************************************************************
 * Copyright (C)
 * File name: 	nodekey_mapping
 * Author: 		Siyuan Sheng
 * Version: 	1
 * Date: 		04/09/2019
 * Description: Maintain a mapping between key and index, which means
 *              you can get key by index or get index by key
**********************************************************************/

#ifndef __NODEKEY_MAPPING_H__
#define __NODEKEY_MAPPING_H__

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <map>
#include <vector>

#include "util.h"
#include "ancillary.h"

#define NODEKEY_MAPPING_LOG_PREFIX "[nodekey_mapping] "

typedef uint32_t index_t;

// keep the mapping between key and index
// T must contains 5 members
// 		var: static uint32_t serialization_size
// 		func: constructor with uint8_t*
// 			  operator < (const&) const
// 			  void serialization(uint8_t*) const
// 			  void to_string(char*) const
template <typename T>
class c_nodekey_mapping_t {
private:
	std::vector<T> index_to_key;
	std::map<T, index_t> key_to_index;

	bool has_key(T key) const;
	
public:
	c_nodekey_mapping_t();
	c_nodekey_mapping_t(const char* load_file);

	void insert_key(T key);
	T get_key_by_index(index_t index) const;
	index_t get_index_by_key(T key) const;
	uint32_t get_number() const;

	void save(const char* save_file) const;
};

// Implementation of template class must be contained in header file
template <typename T>
c_nodekey_mapping_t<T>::c_nodekey_mapping_t() {
	index_to_key.clear();
	key_to_index.clear();
}

template <typename T>
c_nodekey_mapping_t<T>::c_nodekey_mapping_t(const char* load_file) {
	index_to_key.clear();
	key_to_index.clear();

	FILE* load_fd = fopen(load_file, "rb");
	if (load_fd == NULL) {
		LOG_ERR(NODEKEY_MAPPING_LOG_PREFIX "c_nodekey_mapping_t: cannot open %s: %s\n",
				load_file, strerror(errno));
	}

	// load key_num
	uint32_t key_num = 0;
	uint32_t read_cnt = fread(&key_num, sizeof(uint32_t), 1, load_fd);
	if (read_cnt != 1) {
		LOG_ERR(NODEKEY_MAPPING_LOG_PREFIX "c_nodekey_mapping_t: invalid read element count "\
				"%d of key_num which should be %d: %s\n", 
				read_cnt, 1, strerror(errno));
	}

	// load serialization_size
	uint32_t serialization_size = 0;
	read_cnt = fread(&serialization_size, sizeof(uint32_t), 1, load_fd);
	if (read_cnt != 1) {
		LOG_ERR(NODEKEY_MAPPING_LOG_PREFIX "c_nodekey_mapping_t: invalid read element count "\
				"%d of serialization_size which should be %d: %s\n", 
				read_cnt, 1, strerror(errno));
	}

	// load keys
	uint8_t buf[serialization_size];
	memset(buf, 0, serialization_size);
	for (uint32_t i = 0; i < key_num; i++) {
		read_cnt = fread(buf, serialization_size, 1, load_fd);
		if (read_cnt != 1) {
			LOG_ERR(NODEKEY_MAPPING_LOG_PREFIX "c_nodekey_mapping_t: invalid read element count "\
					"%d of key which should be %d: %s\n", 
					read_cnt, 1, strerror(errno));
		}

		T t = T(buf);

		// update index_to_key
		index_to_key.push_back(t);

		// update key_to_index
		std::pair<T, index_t> tmp_pair = std::pair<T, index_t>(t, i);
		if (has_key(t)) {
			char tmp[MAX_PATH_LENGTH];
			t.to_string(tmp);
			LOG_ERR(NODEKEY_MAPPING_LOG_PREFIX "c_nodekey_mapping_t: the key %s already exists\n", tmp);
		}
		key_to_index.insert(tmp_pair);
	}

	fclose(load_fd);
}

template <typename T>
bool c_nodekey_mapping_t<T>::has_key(T key) const {
	if (key_to_index.find(key) != key_to_index.end()) return true;
	else return false;
}

template <typename T>
void c_nodekey_mapping_t<T>::insert_key(T key) {
	if (has_key(key)) return;
	
	index_t index = index_to_key.size();
	index_to_key.push_back(key);

	std::pair<T, index_t> key_index_pair = std::pair<T, index_t>(key, index);
	key_to_index.insert(key_index_pair);
}

template <typename T>
T c_nodekey_mapping_t<T>::get_key_by_index(index_t index) const {
	if (index < 0 || index >= index_to_key.size()) {
		LOG_ERR(NODEKEY_MAPPING_LOG_PREFIX "get_key_by_index: "\
				"Invalid index %d which must be in the range of [0, %ld)\n",
				index, index_to_key.size());
	}

	return index_to_key[index];
}

template <typename T>
index_t c_nodekey_mapping_t<T>::get_index_by_key(T key) const {
	if (!has_key(key)) {
		char tmp[MAX_STRING_LENGTH];
		key.to_string(tmp);
		LOG_ERR(NODEKEY_MAPPING_LOG_PREFIX "get_index_by_key: no such key %s\n", tmp);
	}

	return key_to_index.at(key);
}

template<typename T>
uint32_t c_nodekey_mapping_t<T>::get_number() const {
	return index_to_key.size();
}

// save format: key_num serialization_size key_0 key_1 ... key_(n-1)
template <typename T>
void c_nodekey_mapping_t<T>::save(const char* save_file) const {
	FILE* save_fd = fopen(save_file, "wb");
	if (save_fd == NULL) {
		LOG_ERR(NODEKEY_MAPPING_LOG_PREFIX "save: cannot open %s: %s\n", save_file, strerror(errno));
	}

	// save key_num
	uint32_t key_num = index_to_key.size();
	fwrite(&key_num, sizeof(uint32_t), 1, save_fd);

	// save serialization_size
	uint32_t serialization_size = T::serialization_size;
	fwrite(&serialization_size, sizeof(uint32_t), 1, save_fd);

	uint8_t buf[serialization_size];
	memset(buf, 0, serialization_size);

	// save keys
	for (uint32_t i = 0; i < key_num; i++) {
		// key serialization
		index_to_key[i].serialization(buf);
		// write buf
		fwrite(buf, serialization_size, 1, save_fd);
	}
	fclose(save_fd);
}

#endif
