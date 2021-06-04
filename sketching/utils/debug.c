#include "./debug.h"
#include "../hash/hash.h"

#include <algorithm>

void find_hh_threshold(vector<tuple_key_t> tuplekey_vector) {
	map<tuple_key_t, uint32_t> tmp_map;
	for (uint32_t i = 0; i < tuplekey_vector.size(); i++) {
		if (tmp_map.find(tuplekey_vector[i]) != tmp_map.end()) {
			tmp_map[tuplekey_vector[i]] += 1;
		}
		else {
			tmp_map.insert(std::pair<tuple_key_t, uint32_t>(tuplekey_vector[i], 1));
		}
	}

	vector<uint32_t> tmp_vector;
	for (map<tuple_key_t, uint32_t>::iterator iter = tmp_map.begin(); iter != tmp_map.end(); iter++) {
		tmp_vector.push_back(iter->second);
	}
	sort(tmp_vector.begin(), tmp_vector.end());

	LOG_MSG("50% HH point: %u\n", tmp_vector[uint32_t(tmp_vector.size()*0.5)]);
	LOG_MSG("80% HH point: %u\n", tmp_vector[uint32_t(tmp_vector.size()*0.8)]);
	LOG_MSG("90% HH point: %u\n", tmp_vector[uint32_t(tmp_vector.size()*0.9)]);
	LOG_MSG("100th HH point: %u\n", tmp_vector[tmp_vector.size()-101]);
	LOG_MSG("Max HH point: %u\n", tmp_vector[uint32_t(tmp_vector.size()-1)]);

	uint32_t idx = 0;
	for (uint32_t i = 0; i < tmp_vector.size(); i++) {
		if (tmp_vector[i] >= HH_THRESHOLD) {
			idx = i+1;
			break;
		}
	}
	LOG_MSG("HH=%d: %d%\n", HH_THRESHOLD, 100*idx/tmp_vector.size());
}

void find_hc_threshold(vector<tuple_key_t> tuplekey_vector, vector<tuple_key_t> next_tuplekey_vector) {
	map<tuple_key_t, uint32_t> tmp_map;
	for (uint32_t i = 0; i < tuplekey_vector.size(); i++) {
		if (tmp_map.find(tuplekey_vector[i]) != tmp_map.end()) {
			tmp_map[tuplekey_vector[i]] += 1;
		}
		else {
			tmp_map.insert(std::pair<tuple_key_t, uint32_t>(tuplekey_vector[i], 1));
		}
	}

	map<tuple_key_t, uint32_t> next_tmp_map;
	for (uint32_t i = 0; i < next_tuplekey_vector.size(); i++) {
		if (next_tmp_map.find(next_tuplekey_vector[i]) != next_tmp_map.end()) {
			next_tmp_map[tuplekey_vector[i]] += 1;
		}
		else {
			next_tmp_map.insert(std::pair<tuple_key_t, uint32_t>(next_tuplekey_vector[i], 1));
		}
	}

	for (map<tuple_key_t, uint32_t>::iterator iter = next_tmp_map.begin(); iter != next_tmp_map.end(); iter++) {
		if (tmp_map.find(iter->first) != tmp_map.end()) {
			tmp_map[iter->first] = abs_minus(tmp_map[iter->first], iter->second);
		}
		else {
			tmp_map.insert(std::pair<tuple_key_t, uint32_t>(iter->first, iter->second));
		}
	}

	vector<uint32_t> change_vector;
	for (map<tuple_key_t, uint32_t>::iterator iter = tmp_map.begin(); iter != tmp_map.end(); iter++) {
		change_vector.push_back(iter->second);
	}
	sort(change_vector.begin(), change_vector.end());

	LOG_MSG("50% HC point: %u\n", change_vector[uint32_t(change_vector.size()*0.5)]);
	LOG_MSG("80% HC point: %u\n", change_vector[uint32_t(change_vector.size()*0.8)]);
	LOG_MSG("90% HC point: %u\n", change_vector[uint32_t(change_vector.size()*0.9)]);
	LOG_MSG("100th HC point: %u\n", change_vector[change_vector.size()-101]);
	LOG_MSG("Max HC point: %u\n", change_vector[uint32_t(change_vector.size()-1)]);

	uint32_t idx = 0;
	for (uint32_t i = 0; i < change_vector.size(); i++) {
		if (change_vector[i] >= HC_THRESHOLD) {
			idx = i;
			break;
		}
	}
	LOG_MSG("HC=%d: %d%\n", HC_THRESHOLD, 100*idx/change_vector.size());
}

void count_assumption_violation(vector<tuple_key_t> tuplekey_vector, uint32_t height, uint32_t width) {
	map<tuple_key_t, uint32_t> tmp_map;
	for (uint32_t i = 0; i < tuplekey_vector.size(); i++) {
		if (tmp_map.find(tuplekey_vector[i]) != tmp_map.end()) {
			tmp_map[tuplekey_vector[i]] += 1;
		}
		else {
			tmp_map.insert(std::pair<tuple_key_t, uint32_t>(tuplekey_vector[i], 1));
		}
	}
	
	// Use CBF as an example
	vector<set<tuple_key_t>> tuplekey_sets(width);
	vector<uint32_t> buckets(width, 0);
	for (map<tuple_key_t, uint32_t>::iterator iter = tmp_map.begin(); iter != tmp_map.end(); iter++) {
		for (uint32_t row_idx = 0; row_idx < height; row_idx++) {
			uint32_t hash_idx = myhash(iter->first, row_idx, width).hash_idx;
			buckets[hash_idx] += iter->second;
			tuplekey_sets[hash_idx].insert(iter->first);
		}
	}

	uint32_t phi = 10;
	uint32_t violation_num1 = 0;
	uint32_t violation_num2 = 0;
	uint32_t heavy_bucketnum = 0;
	for (uint32_t i = 0; i < width; i++) {
		if (buckets[i] <= phi) {
			continue;
		}
		heavy_bucketnum += 1;

		set<tuple_key_t> tmp_set = tuplekey_sets[i]; 
		uint32_t tmp_hhnum = 0;
		for (set<tuple_key_t>::iterator iter = tmp_set.begin(); iter != tmp_set.end(); iter++) {
			if (tmp_map[*iter] > phi) {
				tmp_hhnum += 1;
			}
		}
		if (tmp_hhnum >= 2) {
			violation_num1 += 1;
		}
		if (tmp_hhnum == 0)  {
			violation_num2 += 1;
		}
	}

	float violation1_frac = float(violation_num1) / heavy_bucketnum;
	float violation2_frac = float(violation_num2) / heavy_bucketnum;
	LOG_MSG("Violation 1 fraction: %f, violation 2 fraction: %f\n", violation1_frac, violation2_frac);
}

void count_assumption_violation2(vector<tuple_key_t> tuplekey_vector, uint32_t height, uint32_t width) {
	map<tuple_key_t, uint32_t> tmp_map;
	for (uint32_t i = 0; i < tuplekey_vector.size(); i++) {
		if (tmp_map.find(tuplekey_vector[i]) != tmp_map.end()) {
			tmp_map[tuplekey_vector[i]] += 1;
		}
		else {
			tmp_map.insert(std::pair<tuple_key_t, uint32_t>(tuplekey_vector[i], 1));
		}
	}
	
	// Use CBF as an example
	vector<set<tuple_key_t>> tuplekey_sets(width);
	vector<uint32_t> buckets(width, 0);
	uint32_t phi = 10;
	uint32_t keynum = tmp_map.size();
	uint32_t violation_num1 = 0;
	uint32_t violation_num2 = 0;
	for (uint32_t i = 0; i < tuplekey_vector.size(); i++) {
		uint32_t min = 0;
		uint32_t min_idx = 0;
		bool is_first_item = false;
		for (uint32_t row_idx = 0; row_idx < height; row_idx++) {
			uint32_t hash_idx = myhash(tuplekey_vector[i], row_idx, width).hash_idx;

			if (tuplekey_sets[hash_idx].find(tuplekey_vector[i]) == tuplekey_sets[hash_idx].end()) {
				is_first_item = true;
			}
			tuplekey_sets[hash_idx].insert(tuplekey_vector[i]);

			if (row_idx == 0 || min > buckets[hash_idx]) {
				min = buckets[hash_idx];
				min_idx = row_idx;
			}
			buckets[hash_idx] += 1;
		}

		if (is_first_item && min > phi) {
			bool is_violation1 = false;
			set<tuple_key_t> tmp_set = tuplekey_sets[min_idx];	
			for (set<tuple_key_t>::iterator iter = tmp_set.begin(); iter != tmp_set.end(); iter++) {
				if (tmp_map[*iter] > 10) {
					is_violation1 = true;
				}
			}

			if (is_violation1) {
				violation_num1 += 1;
			}
			else {
				violation_num2 += 1;
			}
		}
	}


	float violation1_frac = float(violation_num1) / keynum;
	float violation2_frac = float(violation_num2) / keynum;
	LOG_MSG("Violation 1 fraction: %f, violation 2 fraction: %f\n", violation1_frac, violation2_frac);
}
