#include "./flowradar.h"

#include "../utils/util.h"
#include "../hash/hash.h"

FlowRadarCounter::FlowRadarCounter(){
	pktcnt = 0;
	tuplekey.src_ip = 0;
	tuplekey.dst_ip = 0;
	flowcnt = 0;
}

FlowRadarCounter::FlowRadarCounter(const FlowRadarCounter& other){
	pktcnt = other.pktcnt;
	tuplekey.src_ip = other.tuplekey.src_ip;
	tuplekey.dst_ip = other.tuplekey.dst_ip;
	flowcnt = other.flowcnt;
}

FlowRadar::FlowRadar(uint32_t bf_space, uint32_t measure_space, uint32_t bf_hash_num, uint32_t cbf_hash_num){
	this->bf_hash_num = bf_hash_num;
	bf_bucketnum = bf_space * 32;
	bf_vector.resize(bf_bucketnum, false);

	this->cbf_hash_num = cbf_hash_num;
	//cbf_bucketnum = (measure_space * 32) / (32 + 64 + 24);
	cbf_bucketnum = (measure_space * 32) / (32 + 64 + 32);
	cbf_vector.resize(cbf_bucketnum, FlowRadarCounter());
}

void FlowRadar::update(tuple_key_t cur_key){
	// Update bloom filter
	bool status = true;
	for (uint32_t bf_row_idx = 0; bf_row_idx < bf_hash_num; bf_row_idx++) {
		uint32_t bf_hash_idx = myhash(cur_key, bf_row_idx, bf_bucketnum).hash_idx;
		if (!bf_vector[bf_hash_idx]) {
			status = false;
			bf_vector[bf_hash_idx] = true;
		}
	}

	for (uint32_t cbf_index = 0; cbf_index < cbf_hash_num; cbf_index++) {
		uint32_t cbf_hash_idx = myhash(cur_key, cbf_index, cbf_bucketnum).hash_idx;
		FlowRadarCounter *cur_counter = &cbf_vector[cbf_hash_idx];

		// Update packet count
		cur_counter->pktcnt += 1;

		// New flow
		if (!status) {
			cur_counter->tuplekey.src_ip = cur_counter->tuplekey.src_ip ^ cur_key.src_ip;
			cur_counter->tuplekey.dst_ip = cur_counter->tuplekey.dst_ip ^ cur_key.dst_ip;
			cur_counter->flowcnt += 1;
		}
	}
}

uint32_t FlowRadar::estimate(tuple_key_t cur_key) {
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

set<tuple_key_t> FlowRadar::get_tuplekeys() {
	if (!is_decode) {
		decode();
	}

	set<tuple_key_t> result;
	for (map<tuple_key_t, uint32_t>::iterator iter = decode_result.begin(); iter != decode_result.end(); iter++) {
		result.insert(iter->first);
	}
	return result;
}

uint32_t FlowRadar::get_transmission_pkts() {
	uint32_t nbyte = bf_vector.size()/8 + cbf_vector.size() * 16;
	uint32_t npkt = (nbyte - 1) / 1500 + 1;
	return npkt;
}

uint32_t FlowRadar::get_transmission_bytes() {
	uint32_t nbyte = bf_vector.size()/8 + cbf_vector.size() * 16;
	return nbyte;
}

void FlowRadar::decode(){
	map<tuple_key_t, uint32_t> tmp_decode_result;
	uint32_t decode_iter = 0;
	while (true) {
		int pure_idx = -1;
		for (uint32_t i = 0; i < cbf_vector.size(); i++) {
			if (cbf_vector[i].flowcnt == 1) {
				pure_idx = i;
				break;
			}
		}

		if (pure_idx < 0) {
			LOG_MSG("Finish decoding with %d iterations\n", decode_iter);
			break;
		}

		tuple_key_t tmp_tuple_key;
		tmp_tuple_key.src_ip = cbf_vector[pure_idx].tuplekey.src_ip;
		tmp_tuple_key.dst_ip = cbf_vector[pure_idx].tuplekey.dst_ip;
		uint32_t tmp_value = cbf_vector[pure_idx].pktcnt;
		if (tmp_decode_result.find(tmp_tuple_key) == tmp_decode_result.end()) {
			tmp_decode_result.insert(std::pair<tuple_key_t, uint32_t>(tmp_tuple_key, tmp_value));
		}
		else {
			if (tmp_decode_result[tmp_tuple_key] > tmp_value) {
				tmp_decode_result[tmp_tuple_key] = tmp_value;
			}
		}

		for (uint32_t cbf_index = 0; cbf_index < cbf_hash_num; cbf_index++) {
			uint32_t cbf_hash_idx = myhash(tmp_tuple_key, cbf_index, cbf_bucketnum).hash_idx;
			cbf_vector[cbf_hash_idx].tuplekey.src_ip = cbf_vector[cbf_hash_idx].tuplekey.src_ip ^ tmp_tuple_key.src_ip;
			cbf_vector[cbf_hash_idx].tuplekey.dst_ip = cbf_vector[cbf_hash_idx].tuplekey.dst_ip ^ tmp_tuple_key.dst_ip;
			cbf_vector[cbf_hash_idx].flowcnt -= 1;
			if (cbf_vector[cbf_hash_idx].pktcnt >= tmp_value) {
				cbf_vector[cbf_hash_idx].pktcnt -= tmp_value;
			}
			else {
				cbf_vector[cbf_hash_idx].pktcnt = 0;
				tmp_decode_result[tmp_tuple_key] = cbf_vector[cbf_hash_idx].pktcnt;
			}
		}
		decode_iter += 1;
	}

	for (map<tuple_key_t, uint32_t>::iterator iter = tmp_decode_result.begin(); iter != tmp_decode_result.end(); iter++) {
		if (iter->second > 0) {
			decode_result.insert(std::pair<tuple_key_t, uint32_t>(iter->first, iter->second));
		}
	}

	is_decode = true;
}
