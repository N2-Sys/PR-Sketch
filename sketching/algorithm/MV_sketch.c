#include "./MV_sketch.h"

#include "../utils/util.h"
#include "../hash/hash.h"

MVSketch::MVSketch(uint32_t total_bytes, uint32_t height) {
	uint32_t nbucket = total_bytes / 16;
	this->width = nbucket / height;
	this->height = height;

	MVSketchBucket tmpbucket;
	tmpbucket.V = 0;
	tmpbucket.C = 0;
	tmpbucket.tuplekey.src_ip = 0;
	tmpbucket.tuplekey.dst_ip = 0;
	buckets.resize(this->width*height, tmpbucket);
}

void MVSketch::update(tuple_key_t cur_key) {
	for (uint32_t i = 0; i < height; i++) {
		uint32_t hashidx = myhash(cur_key, i, width).hash_idx;
		MVSketchBucket* cur_bucket = get_bucket(i, hashidx);

		cur_bucket->V += 1;
		if (cur_bucket->tuplekey.src_ip==0 && cur_bucket->tuplekey.dst_ip==0) {
			cur_bucket->C = 1;
			cur_bucket->tuplekey = cur_key;
		}
		else if (cur_bucket->tuplekey == cur_key) {
			cur_bucket->C += 1;
		}
		else {
			cur_bucket->C -= 1;
			if (cur_bucket->C < 0) {
				cur_bucket->tuplekey = cur_key;
				cur_bucket->C = (-cur_bucket->C);
			}
		}
	}
}

uint32_t MVSketch::estimate(tuple_key_t cur_key) {
	uint32_t min_value = 0;
	for (uint32_t i = 0; i < height; i++) {
		uint32_t hashidx = myhash(cur_key, i, width).hash_idx;
		MVSketchBucket* cur_bucket = get_bucket(i, hashidx);
		uint32_t tmp_value;
		if (cur_bucket->tuplekey == cur_key) {
			tmp_value = (cur_bucket->V + cur_bucket->C) / 2;
		}
		else {
			tmp_value = (cur_bucket->V - cur_bucket->C) / 2;
		}
		if (i == 0 || tmp_value < min_value) {
			min_value = tmp_value;
		}
	}
	return min_value;
}

set<tuple_key_t> MVSketch::get_tuplekeys() {
	set<tuple_key_t> result;
	for (uint32_t i = 0; i < height; i++) {
		for (uint32_t j = 0; j < width; j++) {
			MVSketchBucket* cur_bucket = get_bucket(i, j);
			if (!(cur_bucket->tuplekey.src_ip == 0 && cur_bucket->tuplekey.dst_ip == 0)) {
				result.insert(cur_bucket->tuplekey);
			}
		}
	}
	return result;
}

uint32_t MVSketch::get_transmission_pkts() {
	uint32_t nbucket = buckets.size();
	uint32_t nbytes = nbucket * 16;
	uint32_t npkts = (nbytes-1)/1500 + 1;
	return npkts;
}

uint32_t MVSketch::get_transmission_bytes() {
	uint32_t nbucket = buckets.size();
	uint32_t nbytes = nbucket * 16;
	return nbytes;
}

MVSketchBucket* MVSketch::get_bucket(uint32_t row, uint32_t col) {
	afs_assert(row >= 0 && row < height, "Invalid row: %d (%d)\n", row, height);
	afs_assert(col >= 0 && col < width, "Invalid col: %d (%d)\n", col, width);
	return &buckets[row*width + col];
}
