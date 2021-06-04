
#include <string.h>
#include "./deltoid.h"

#include "../utils/util.h"
#include "../hash/hash.h"

Deltoid::Deltoid(uint32_t total_bytes, uint32_t height, uint32_t keylen, uint32_t threshold) {
	this->height = height;
	this->keylen = keylen;
	this->threshold = threshold;
	this->width = total_bytes / 4 / height / (keylen + 1);

	vector<uint32_t> tmpgroup(keylen+1, 0);
	groups.resize(this->width*height, tmpgroup);
}

void Deltoid::update(tuple_key_t cur_key) {
	uint32_t nbyte = (keylen-1) / BYTESIZE + 1;
	uint8_t *bytes = (uint8_t *)(&cur_key);
	for (uint32_t i = 0; i < height; i++) {
		uint32_t hashidx = myhash(cur_key, i, width).hash_idx;
		vector<uint32_t>* cur_group = get_group(i, hashidx);

		(*cur_group)[0] += 1;
		for (uint32_t byteidx = 0; byteidx < nbyte; byteidx++) {
			uint8_t tmpmask = 1;
			for (uint32_t bitidx = 0; bitidx < BYTESIZE; bitidx++) {
				if ((bytes[byteidx] & tmpmask) == tmpmask) {
					(*cur_group)[byteidx*BYTESIZE+bitidx+1] += 1;
				}
				tmpmask = tmpmask << 1;
			}
		}
	}
}

uint32_t Deltoid::estimate(tuple_key_t cur_key) {
	if (!is_decode) {
		decode();
	}

	if (decode_result.find(cur_key) != decode_result.end()) {
		return decode_result[cur_key];
	}
	else {
		return 0;
	}
}

set<tuple_key_t> Deltoid::get_tuplekeys() {
	if (!is_decode) {
		decode();
	}
	return decode_tuplekeys;
}

uint32_t Deltoid::get_transmission_pkts() {
	uint32_t nbucket = height * width * (keylen + 1);
	uint32_t nbyte = nbucket * 4;
	uint32_t npkt = (nbyte-1)/1500 + 1;
	return npkt;
}
uint32_t Deltoid::get_transmission_bytes() {
	uint32_t nbucket = height * width * (keylen + 1);
	uint32_t nbyte = nbucket * 4;
	return nbyte;
}

vector<uint32_t>* Deltoid::get_group(uint32_t row, uint32_t col) {
	afs_assert(row >= 0 && row < height, "Invalid row: %d (%d)\n", row, height);
	afs_assert(col >= 0 && col < width, "Invalid col: %d (%d)\n", col, width);
	return &groups[row*width+col];
}

void Deltoid::decode() {
	uint32_t nbyte = (keylen-1) / BYTESIZE + 1;
	for (uint32_t i = 0; i < height; i++) {
		for (uint32_t j = 0; j < width; j++) {
			vector<uint32_t>* cur_group = get_group(i, j);

			// judge whether there is a valid deltoid by threshold
			bool is_valid = true;
			uint8_t cur_bytes[nbyte];
			memset(cur_bytes, 0, nbyte);

			//uint32_t totalv = (*cur_group)[0];
			for (uint32_t k = 1; k < keylen+1; k++) {
				uint32_t localv = (*cur_group)[k];
				/*if (((localv < threshold) && ((totalv - localv) < threshold)) ||
						((localv >= threshold) && ((totalv - localv) >= threshold))) {
					is_valid = false;
					break;
				}*/

				if (localv >= threshold) {
					uint32_t byteidx = (k-1) / BYTESIZE;
					uint32_t bitidx = (k-1) - byteidx * BYTESIZE;
					uint8_t mask = 1 << bitidx;
					cur_bytes[byteidx] = cur_bytes[byteidx] | mask;
				}
			}
			if (!is_valid) {
				continue;
			}

			tuple_key_t* cur_key = (tuple_key_t*)cur_bytes;
			if (cur_key->src_ip == 0 && cur_key->dst_ip == 0) {
				continue;
			}

			// check whether this key can be hashed into this group
			uint32_t hashidx = myhash(*cur_key, i, width).hash_idx;
			if (hashidx != j) {
				continue;
			}

			// check whether it has been extracted before
			if (decode_tuplekeys.find(*cur_key) != decode_tuplekeys.end()) {
				continue;
			}
			decode_tuplekeys.insert(*cur_key);

			// get the minimum value as the estimation
			uint32_t min_value = 0;
			for (uint32_t rowidx = 0; rowidx < height; rowidx++) {
				hashidx = myhash(*cur_key, rowidx, width).hash_idx;
				uint32_t tmp_value = (*get_group(rowidx, hashidx))[0];
				if (rowidx == 0 || tmp_value < min_value) {
					min_value = tmp_value;
				}
			}
			decode_result[*cur_key] = min_value;
		}
	}
	is_decode = true;
}
