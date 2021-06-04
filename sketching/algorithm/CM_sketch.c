#include "./CM_sketch.h"

#include "../utils/util.h"
#include "../hash/hash.h"

CMSketch::CMSketch(uint32_t total_bytes, uint32_t height) {
	uint32_t nbucket = total_bytes / 4;
	this->width = nbucket / height;
	this->height = height;

	buckets.resize(this->width*height, 0);
}

void CMSketch::update(tuple_key_t cur_key) {
	for (uint32_t i = 0; i < height; i++) {
		uint32_t hashidx = myhash(cur_key, i, width).hash_idx;
		uint32_t* cur_bucket = get_bucket(i, hashidx);
		*cur_bucket += 1;
	}
}

uint32_t CMSketch::estimate(tuple_key_t cur_key) {
	uint32_t min_value = 0;
	for (uint32_t i = 0; i < height; i++) {
		uint32_t hashidx = myhash(cur_key, i, width).hash_idx;
		uint32_t* cur_bucket = get_bucket(i, hashidx);
		if (i == 0 || *cur_bucket < min_value) {
			min_value = *cur_bucket;
		}
	}
	return min_value;
}

set<tuple_key_t> CMSketch::get_tuplekeys() {
	set<tuple_key_t> result;
	return result;
}

uint32_t CMSketch::get_transmission_pkts() {
	uint32_t nbucket = buckets.size();
	uint32_t nbytes = nbucket * 4;
	uint32_t npkts = (nbytes-1)/1500 + 1;
	return npkts;
}

uint32_t CMSketch::get_transmission_bytes(){
	uint32_t nbucket = buckets.size();
	uint32_t nbytes = nbucket * 4;
	return nbytes;
}

uint32_t* CMSketch::get_bucket(uint32_t row, uint32_t col) {
	afs_assert(row >= 0 && row < height, "Invalid row: %d (%d)\n", row, height);
	afs_assert(col >= 0 && col < width, "Invalid col: %d (%d)\n", col, width);
	return &buckets[row*width + col];
}
