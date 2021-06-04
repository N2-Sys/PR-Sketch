#ifndef UNIVMON_H
#define UNIVMON_H

#include <iostream>
#include <vector>
#include <map>

#include "../tuple/tuple.h"
#include "./countheap.h"

using namespace std;

class Univmon: public BaseSketch{
	public:
		Univmon(uint32_t total_bytes, uint32_t layernum, uint32_t height, uint32_t heapsize_per_layer);

		void update(tuple_key_t cur_key);
		void update(tuple_key_t cur_key, bool is_ns, uint32_t ns_row, uint32_t ns_value);
		uint32_t estimate(tuple_key_t cur_key);
		set<tuple_key_t> get_tuplekeys();
		uint32_t get_transmission_pkts();
		uint32_t get_transmission_bytes();
	private:
		uint32_t layernum;

		vector<CountHeap> countheap_vector;

		bool is_decode = false;
		set<tuple_key_t> decode_tuplekeys;
		map<tuple_key_t, uint32_t> decode_result;
		void decode();
};

#endif
