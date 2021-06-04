#include "./lossy_counting.h"
#include "../hash/hash.h"

#include <math.h>

LossyCounting::LossyCounting(uint32_t total_bytes, float epsilon) {
	this->epsilon = epsilon;
	cur_packetnum = 0;
	prev_bucketidx = 0;

	uint32_t entry_size = 8 + 4 + 4; // flowkey, frequency, bucketidx
	max_datasize = total_bytes / entry_size;

	tuple_key_t invalid_tuplekey;
	invalid_tuplekey.src_ip = 0;
	invalid_tuplekey.dst_ip = 0;
	tuplekeys.resize(max_datasize, invalid_tuplekey);
	frequencies.resize(max_datasize, 0);
	bucketidxes.resize(max_datasize, 0);
}

void LossyCounting::update(tuple_key_t cur_key) {
	cur_packetnum += 1;
	uint32_t cur_bucketidx = ceil(cur_packetnum * epsilon);

	// Clear data
	if (cur_bucketidx > prev_bucketidx) {
		/*for (map<tuple_key_t, std::pair<uint32_t, uint32_t>>::iterator iter = data.begin(); iter != data.end(); ) {
			if ((iter->second.first + iter->second.second) <= cur_bucketidx) { // frequency + bucketidx <= curremt bucket idx
				data.erase(iter++);
			}
			else {
				iter++;
			}
		}*/
		for (uint32_t i = 0; i < max_datasize; i++) {
			if (frequencies[i] > 0 && (frequencies[i]+bucketidxes[i]) <= cur_bucketidx) {
				tuplekeys[i].src_ip = 0;
				tuplekeys[i].dst_ip = 0;
				frequencies[i] = 0;
				bucketidxes[i] = 0;
			}
		}
	}

	// Update data
	/*if (data.find(cur_key) != data.end()) {
		data[cur_key].first += 1; // frequency ++
	}
	else {
		if (data.size() < max_datasize) { // Not full
			// Add entry <flowkey, 1, current bucketidx - 1>
			data.insert(std::pair<tuple_key_t, std::pair<uint32_t, uint32_t>>(\
						cur_key, \
						std::pair<uint32_t, uint32_t>(1, cur_bucketidx - 1)));
		}
	}*/
	uint32_t hashidx = myhash(cur_key, 0, max_datasize).hash_idx;
	if (tuplekeys[hashidx] == cur_key) {
		frequencies[hashidx] += 1;
	}
	else {
		if (frequencies[hashidx] == 0) {
			tuplekeys[hashidx] = cur_key;
			frequencies[hashidx] = 1;
			bucketidxes[hashidx] = cur_bucketidx - 1;
		}
	}

	prev_bucketidx = cur_bucketidx;
}

uint32_t LossyCounting::estimate(tuple_key_t cur_key) {
	/*if (data.find(cur_key) != data.end()) {
		return data[cur_key].first;
	}
	else {
		return 0;
	}*/
	uint32_t hashidx = myhash(cur_key, 0, max_datasize).hash_idx;
	if (tuplekeys[hashidx] == cur_key) {
		return frequencies[hashidx];
	}
	else {
		return 0;
	}
}

set<tuple_key_t> LossyCounting::get_tuplekeys() {
	set<tuple_key_t> result;	
	/*for (map<tuple_key_t, std::pair<uint32_t, uint32_t>>::iterator iter = data.begin(); iter != data.end(); ) {
		result.insert(iter->first);
	}*/
	for (uint32_t i = 0; i < max_datasize; i++) {
		if (frequencies[i] > 0) {
			result.insert(tuplekeys[i]);
		}
	}
	return result;
}

uint32_t LossyCounting::get_transmission_pkts() {
	uint32_t entry_size = 8 + 4 + 4; // flowkey, frequency, bucketidx
	uint32_t nbytes = max_datasize * entry_size;
	uint32_t npkts = (nbytes - 1)/1500 + 1;
	return npkts;
}

uint32_t LossyCounting::get_transmission_bytes() {
	uint32_t entry_size = 8 + 4 + 4; // flowkey, frequency, bucketidx
	uint32_t nbytes = max_datasize * entry_size;
	return nbytes;
}
