#ifndef FLOWSIZE_METRIC_H
#define FLOWSIZE_METRIC_H

#include <iostream>
#include <map>
#include <vector>
#include <set>

#include "../tuple/tuple.h"
#include "../utils/util.h"
#include "../utils/estimator.h"

using namespace std;

class FlowsizeMetric {
	public:
		FlowsizeMetric(vector<tuple_key_t> tuplekey_vector);
		FlowsizeMetric(const map<tuple_key_t, uint32_t>& kv_map);
		FlowsizeMetric(const map<uint32_t, uint32_t>& flowsize_map);

		void dump(const char* filename);
		float get_WMRD(const FlowsizeMetric& other);
	private:
		map<uint32_t, uint32_t> flowsize_map;
		void generate_flowsize_map(const map<tuple_key_t, uint32_t>& kv_map);
};

#endif
