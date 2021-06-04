#include "./elastic_sketch.h"

#include "../utils/util.h"
#include "../hash/hash.h"

ElasticSketchHeavyCounter::ElasticSketchHeavyCounter(uint32_t len) {
	pvotes.resize(len, 0);

	tuple_key_t tmp;
	tmp.src_ip = 0;
	tmp.dst_ip = 0;
	tuplekeys.resize(len, tmp);

	flags.resize(len, false);

	nvote = 0;
}

ElasticSketch::ElasticSketch(uint32_t filter_space, uint32_t measure_space, uint32_t counter_len, uint32_t height) {
	this->height = height; // hash num of light part
	this->counter_len = counter_len;

	heavy_bucketnum = filter_space * 32 / (97 * counter_len + 32);	// 4 ints is the size of heavy counter
	heavy_counter_vector.resize(heavy_bucketnum, ElasticSketchHeavyCounter(counter_len));

	width = measure_space / height; // 1 int is the size of light part counter
	light_vector.resize(height*width, 0);
}

void ElasticSketch::update(tuple_key_t cur_key) {
	// Update hash table in heavy part
	uint32_t heavy_hash_idx = myhash(cur_key, 0, heavy_bucketnum).hash_idx;
	ElasticSketchHeavyCounter *cur_counter = &heavy_counter_vector[heavy_hash_idx];
	uint32_t min_idx = 0;
	uint32_t min_data = 0;
	for (uint32_t i = 0; i < counter_len; i++) {
		if (cur_counter->tuplekeys[i].src_ip == 0 && cur_counter->tuplekeys[i].dst_ip == 0) {
			cur_counter->tuplekeys[i].src_ip = cur_key.src_ip;
			cur_counter->tuplekeys[i].dst_ip = cur_key.dst_ip;
			cur_counter->pvotes[i] = 1;
			return;
		}
		else if (cur_counter->tuplekeys[i].src_ip == cur_key.src_ip && cur_counter->tuplekeys[i].dst_ip == cur_key.dst_ip) {
			cur_counter->pvotes[i] += 1;
			return;
		}
		
		if (i == 0 || cur_counter->pvotes[i] < min_data) {
			min_idx = i;
			min_data = cur_counter->pvotes[i];
		}
	}
	cur_counter->nvote += 1;

	tuple_key_t tmp_tuplekey = cur_key;
	uint32_t tmp_value = 1;
	if ((cur_counter->nvote + 1) > (8*min_data)) {
		tmp_tuplekey.src_ip = cur_counter->tuplekeys[min_idx].src_ip;
		tmp_tuplekey.dst_ip = cur_counter->tuplekeys[min_idx].dst_ip;
		tmp_value = min_data;

		// Save small flow
		cur_counter->tuplekeys[min_idx].src_ip = cur_key.src_ip;
		cur_counter->tuplekeys[min_idx].dst_ip = cur_key.dst_ip;
		cur_counter->pvotes[min_idx] = 1;
		cur_counter->flags[min_idx] = true;
		cur_counter->nvote = 0;
	}

	// Update light part counter
	for (uint32_t row_idx = 0; row_idx < height; row_idx++) {
		hash_result_t hash_result = myhash(tmp_tuplekey, row_idx, width);
		uint32_t* cur_count = get_light_count(row_idx, hash_result.hash_idx);
		(*cur_count) += tmp_value;
	}

}

uint32_t ElasticSketch::estimate(tuple_key_t cur_key) {
	uint32_t result = 0;

	// Check heavy part of Elastic Sketch
	uint32_t heavy_hash_idx = myhash(cur_key, 0, heavy_bucketnum).hash_idx;
	ElasticSketchHeavyCounter *cur_counter = &heavy_counter_vector[heavy_hash_idx];
	bool flag = false;
	for (uint32_t i = 0; i < counter_len; i++) {
		if (cur_counter->tuplekeys[i].src_ip == cur_key.src_ip && cur_counter->tuplekeys[i].dst_ip == cur_key.dst_ip) {
			result = cur_counter->pvotes[i];
			flag = cur_counter->flags[i];
			break;
		}
	}

	if (flag) {
		uint32_t min_value = 0;
		for (uint32_t row_idx = 0; row_idx < height; row_idx++) {
			hash_result_t hash_result = myhash(cur_key, row_idx, width);
			uint32_t* cur_count = get_light_count(row_idx, hash_result.hash_idx);
			if (row_idx == 0 || (*cur_count) < min_value) {
				min_value = (*cur_count);
			}
		}
		result += min_value;
	}
	return result;

}

set<tuple_key_t> ElasticSketch::get_tuplekeys() {
	set<tuple_key_t> result;
	for (uint32_t i = 0; i < heavy_counter_vector.size(); i++) {
		ElasticSketchHeavyCounter cur_counter = heavy_counter_vector[i];
		for (uint32_t j = 0; j < counter_len; j++) {
			if (cur_counter.tuplekeys[j].src_ip != 0 && cur_counter.tuplekeys[j].dst_ip != 0) {
				result.insert(cur_counter.tuplekeys[j]);
			}
		}
	}
	return result;
}

uint32_t ElasticSketch::get_transmission_pkts() {
	uint32_t heavy_nbucket = heavy_counter_vector.size();
	uint32_t light_nbucket = light_vector.size();
	uint32_t nbytes = heavy_nbucket * (97 * counter_len + 32) / 8;
	nbytes += light_nbucket * 4;
	uint32_t npkts = (nbytes-1)/1500 + 1;
	return npkts;
}

uint32_t ElasticSketch::get_transmission_bytes() {
	uint32_t heavy_nbucket = heavy_counter_vector.size();
	uint32_t light_nbucket = light_vector.size();
	uint32_t nbytes = heavy_nbucket * (97 * counter_len + 32) / 8;
	nbytes += light_nbucket * 4;
	return nbytes;
}

uint32_t* ElasticSketch::get_light_count(uint32_t row, uint32_t col) {
	afs_assert(row >= 0 && row < height, "Invalid row: %d (%d)\n", row, height);
	afs_assert(col >= 0 && col < width, "Invalid col: %d (%d)\n", col, width);
	uint32_t index = row*width + col;
	return &light_vector[index];
}
