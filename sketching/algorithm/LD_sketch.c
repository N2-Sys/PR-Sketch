#include "./LD_sketch.h"

#include "../utils/util.h"
#include "../hash/hash.h"

uint32_t LDSketch::max_nbucket = 0;
uint32_t LDSketch::max_length = 0;
uint32_t LDSketch::T = 0;

uint32_t LDSketchBucket::bucket_update(tuple_key_t cur_key, uint32_t cur_nbucket) {
	V += 1;

	if (array.find(cur_key) != array.end()) {
		array[cur_key] += 1;
	}
	else if (array.size() < length) {
		array.insert(pair<tuple_key_t, uint32_t>(cur_key, 1));
	}
	else {
		uint32_t k = V / LDSketch::T;
		if (((k+1)*(k+2)-1) <= length) {
			uint32_t tmp_min = 0;
			for (map<tuple_key_t, uint32_t>::iterator iter = array.begin(); iter != array.end(); iter++) {
				if (iter == array.begin()) {
					tmp_min = iter->second;
				}
				else {
					if (iter->second < tmp_min) {
						tmp_min = iter->second;
					}
				}
			}

			if (1 < tmp_min) {
				tmp_min = 1;
			}
			e += tmp_min;

			for (map<tuple_key_t, uint32_t>::iterator iter = array.begin(); iter != array.end(); ) {
				iter->second -= tmp_min;
				if (iter->second == 0) {
					map<tuple_key_t, uint32_t>::iterator tmp_iter = next(iter);
					array.erase(iter); // Invalidate iter
					iter = tmp_iter;
				}
				else {
					iter++;
				}
			}

			if (1 > tmp_min) {
				array.insert(pair<tuple_key_t, uint32_t>(cur_key, 1-tmp_min));
			}
		}
		else if (cur_nbucket < LDSketch::max_nbucket) {
			uint32_t prev_length = length;
			length = (k+1)*(k+2) - 1;
			if (length > LDSketch::max_length) {
				length = LDSketch::max_length;
			}

			uint32_t delta_length = length - prev_length;
			if (delta_length > LDSketch::max_nbucket - cur_nbucket) {
				length = prev_length + LDSketch::max_nbucket - cur_nbucket;
				delta_length = LDSketch::max_nbucket - cur_nbucket;
			}

			array.insert(pair<tuple_key_t, uint32_t>(cur_key, 1));
			return delta_length;
		}
	}
	return 0;
}

uint32_t LDSketchBucket::bucket_estimate(tuple_key_t cur_key) {
	// lower bound
	uint32_t result = bucket_estimate_lowerbound(cur_key);

	// upper bound
	result += e;
	return result;
}

uint32_t LDSketchBucket::bucket_estimate_lowerbound(tuple_key_t cur_key) {
	// lower bound
	uint32_t result = 0;
	if (array.size() > 0) {
		if (array.find(cur_key) != array.end()) {
			result = array[cur_key];
		}
	}
	return result;
}

vector<tuple_key_t> LDSketchBucket::get_bucket_tuplekeys() {
	vector<tuple_key_t> result;
	for (map<tuple_key_t, uint32_t>::iterator iter = array.begin(); iter != array.end(); iter++) {
		result.push_back(iter->first);
	}
	return result;
}

LDSketch::LDSketch(uint32_t total_bytes, uint32_t hashnum, uint32_t threshold, float ratio) {
	this->height = hashnum;

	uint32_t sizeofbucket = 12*1 + 4 + 4;
	uint32_t bucketnum = total_bytes * ratio / sizeofbucket;
	this->width = bucketnum / height;

	LDSketch::max_nbucket = total_bytes * (1-ratio) / 12;
	LDSketch::max_length = 10;
	LDSketch::T = threshold; // HH
	// LDSketch::T = 0.5 * threshold; // HC
	this->cur_nbucket = 0;
	
	buckets.resize(this->width*height, LDSketchBucket());
}

void LDSketch::update(tuple_key_t cur_key) {
	for (uint32_t i = 0; i < height; i++) {
		uint32_t hashidx = myhash(cur_key, i, width).hash_idx;
		LDSketchBucket* tmpbkt = get_bucket(i, hashidx);
		uint32_t delta = tmpbkt->bucket_update(cur_key, cur_nbucket);
		cur_nbucket += delta;
	}
}

uint32_t LDSketch::estimate(tuple_key_t cur_key) {
	uint32_t result = 0;
	for (uint32_t i = 0; i < height; i++) {
		uint32_t hashidx = myhash(cur_key, i, width).hash_idx;
		LDSketchBucket* tmpbkt = get_bucket(i, hashidx);
		uint32_t tmpresult = tmpbkt->bucket_estimate(cur_key);
		if (i == 0 || tmpresult < result) {
			result = tmpresult;
		}
	}
	return result;
}

uint32_t LDSketch::estimate_lowerbound(tuple_key_t cur_key) {
	uint32_t result = 0;
	for (uint32_t i = 0; i < height; i++) {
		uint32_t hashidx = myhash(cur_key, i, width).hash_idx;
		LDSketchBucket* tmpbkt = get_bucket(i, hashidx);
		uint32_t tmpresult = tmpbkt->bucket_estimate_lowerbound(cur_key);
		if (i == 0 || tmpresult > result) {
			result = tmpresult;
		}
	}
	return result;
}

set<tuple_key_t> LDSketch::get_tuplekeys() {
	set<tuple_key_t> result;
	for (uint32_t i = 0; i < height; i++) {
		for (uint32_t j =0; j < width; j++) {
			LDSketchBucket* tmpbkt = get_bucket(i, j);
			if (tmpbkt->length > 0) {
				vector<tuple_key_t> tmp_tuplekeys = tmpbkt->get_bucket_tuplekeys();
				for (uint32_t k = 0; k < tmp_tuplekeys.size(); k++) {
					result.insert(tmp_tuplekeys[k]);
				}
			}
		}
	}
	return result;
}

uint32_t LDSketch::get_transmission_pkts() {
	uint32_t nbucket = buckets.size();
	uint32_t sizeofbucket = 12*1 + 4 + 4;
	uint32_t nbytes = nbucket * sizeofbucket + cur_nbucket * 12;
	uint32_t npkts = (nbytes-1)/1500 + 1;
	return npkts;
}

uint32_t LDSketch::get_transmission_bytes() {
	uint32_t nbucket = buckets.size();
	uint32_t sizeofbucket = 12*1 + 4 + 4;
	uint32_t nbytes = nbucket * sizeofbucket + cur_nbucket * 12;
	return nbytes;
}

set<tuple_key_t> LDSketch::get_hcs(BaseSketch* other, uint32_t threshold) {
	afs_assert(other!=NULL, "NULL pointer of next_sketch for LD sketch!\n");
	LDSketch* next_sketch = (LDSketch*)(other);
	set<tuple_key_t> tuplekey_set = get_tuplekeys();
	set<tuple_key_t> next_tuplekey_set = next_sketch->get_tuplekeys();

	map<tuple_key_t, uint32_t> upperbounds;
	map<tuple_key_t, uint32_t> lowerbounds;
	for (set<tuple_key_t>::iterator iter = tuplekey_set.begin(); iter != tuplekey_set.end(); iter++) {
		upperbounds.insert(std::pair<tuple_key_t, uint32_t>(*iter, estimate(*iter)));
		lowerbounds.insert(std::pair<tuple_key_t, uint32_t>(*iter, estimate_lowerbound(*iter)));
	}

	map<tuple_key_t, uint32_t> next_upperbounds;
	map<tuple_key_t, uint32_t> next_lowerbounds;
	for (set<tuple_key_t>::iterator iter = next_tuplekey_set.begin(); iter != next_tuplekey_set.end(); iter++) {
		next_upperbounds.insert(std::pair<tuple_key_t, uint32_t>(*iter, next_sketch->estimate(*iter)));
		next_lowerbounds.insert(std::pair<tuple_key_t, uint32_t>(*iter, next_sketch->estimate_lowerbound(*iter)));
	}

	set<tuple_key_t> result;
	for (set<tuple_key_t>::iterator iter = next_tuplekey_set.begin(); iter != next_tuplekey_set.end(); iter++) {
		if (tuplekey_set.find(*iter) == tuplekey_set.end()) {
			if (next_upperbounds[*iter] >= threshold) {
				result.insert(*iter);
			}
		}
		else {
			if (abs_minus(next_upperbounds[*iter], lowerbounds[*iter]) >= threshold || \
					abs_minus(next_lowerbounds[*iter], upperbounds[*iter]) >= threshold) {
				result.insert(*iter);
			}
		}
	}
	for (set<tuple_key_t>::iterator iter = tuplekey_set.begin(); iter != tuplekey_set.end(); iter++) {
		if (next_tuplekey_set.find(*iter) == next_tuplekey_set.end()) {
			if (upperbounds[*iter] >= threshold) {
				result.insert(*iter);
			}
		}
	}
	return result;
}

LDSketchBucket* LDSketch::get_bucket(uint32_t row, uint32_t col) {
	afs_assert(row >= 0 && row < height, "Invalid row: %d (%d)\n", row, height);
	afs_assert(col >= 0 && col < width, "Invalid col: %d (%d)\n", col, width);
	uint32_t idx = row * width + col;
	return &buckets[idx];
}
