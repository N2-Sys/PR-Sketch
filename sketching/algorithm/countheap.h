#ifndef COUNTHEAP_H
#define COUNTHEAP_H

#include <iostream>
#include <vector>
#include <map>

#include "../tuple/tuple.h"
#include "./base_sketch.h"

using namespace std;

struct mycmp{
	public:
		bool operator ()(const std::pair<tuple_key_t, uint32_t>& a, const std::pair<tuple_key_t, uint32_t>& b) {
			return a.second < b.second;
		}
};

class CountHeap: public BaseSketch{
	public:
		CountHeap(uint32_t total_bytes, uint32_t height, uint32_t heapsize, uint32_t layer_index=0);
		CountHeap(const CountHeap& other);

		void update(tuple_key_t cur_key);
		void update(tuple_key_t cur_key, bool is_ns, uint32_t ns_row, uint32_t ns_value); // For NS
		uint32_t estimate(tuple_key_t cur_key);
		set<tuple_key_t> get_tuplekeys();
		uint32_t get_transmission_pkts();
		uint32_t get_transmission_bytes();

		vector< std::pair<tuple_key_t, uint32_t> > get_heap();
	private:
		uint32_t height;
		uint32_t width;
		uint32_t layer_index;
		uint32_t heapsize;

		vector<int32_t> buckets;
		vector< std::pair<tuple_key_t, uint32_t> > heap_vector;
		//map<tuple_key_t, uint32_t> heap_map;

		int32_t* get_bucket(uint32_t row, uint32_t col);
};

#endif
