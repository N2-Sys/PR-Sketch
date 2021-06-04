#include <stdio.h>

#include "./flowsize_metric.h"

FlowsizeMetric::FlowsizeMetric(vector<tuple_key_t> tuplekey_vector) {
	map<tuple_key_t, uint32_t> kv_map;
	for (uint32_t i = 0; i < tuplekey_vector.size(); i++) {
		if (kv_map.find(tuplekey_vector[i]) == kv_map.end()) {
			kv_map[tuplekey_vector[i]] = 1;
		}
		else {
			kv_map[tuplekey_vector[i]] += 1;
		}
	}
	generate_flowsize_map(kv_map);
}

FlowsizeMetric::FlowsizeMetric(const map<tuple_key_t, uint32_t>& kv_map) {
	generate_flowsize_map(kv_map);
}

FlowsizeMetric::FlowsizeMetric(const map<uint32_t, uint32_t>& flowsize_map) {
	this->flowsize_map = flowsize_map;
}

void FlowsizeMetric::generate_flowsize_map(const map<tuple_key_t, uint32_t>& kv_map) {
	for (map<tuple_key_t, uint32_t>::const_iterator iter = kv_map.begin(); iter != kv_map.end(); iter++) {
		if (flowsize_map.find(iter->second) == flowsize_map.end()) {
			flowsize_map[iter->second] = 1;
		}
		else {
			flowsize_map[iter->second] += 1;
		}
	}
}

void FlowsizeMetric::dump(const char* filename) {
	FILE* fd = fopen_with_dirs(filename, "w+");

	fprintf(fd, "%lu\n", flowsize_map.size());
	for (map<uint32_t, uint32_t>::iterator iter = flowsize_map.begin(); iter != flowsize_map.end(); iter++) {
		fprintf(fd, "%d %d\n", iter->first, iter->second);
	}

	fclose(fd);
}

float FlowsizeMetric::get_WMRD(const FlowsizeMetric& other) {
	float numerator = 0.0;
	float denominator = 0.0;
	map<uint32_t, std::pair<uint32_t, uint32_t>> tmp_map;
	for (map<uint32_t, uint32_t>::iterator iter = flowsize_map.begin(); iter != flowsize_map.end(); iter++) {
		tmp_map[iter->first] = std::pair<uint32_t, uint32_t> (iter->second, 0);
	}
	for (map<uint32_t, uint32_t>::const_iterator iter = other.flowsize_map.begin(); iter != other.flowsize_map.end(); iter++) {
		if (tmp_map.find(iter->first) != tmp_map.end()) {
			tmp_map[iter->first].second = iter->second;
		}
		else {
			tmp_map[iter->first] = std::pair<uint32_t, uint32_t> (0, iter->second);
		}
	}

	for (map<uint32_t, std::pair<uint32_t, uint32_t>>::iterator iter = tmp_map.begin(); iter != tmp_map.end(); iter++) {
		uint32_t a = iter->second.first;
		uint32_t b = iter->second.second;
		numerator += float(abs_minus(a, b));
		denominator += float(a + b) / 2.0;
	}
	return numerator / denominator;
}
