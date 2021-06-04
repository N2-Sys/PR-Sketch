#ifndef CMHeap_H
#define CMHeap_H

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

class CMHeap: public BaseSketch{
	public:
		CMHeap(uint32_t total_bytes, uint32_t height, uint32_t heapsize, uint32_t threshold);

		void update(tuple_key_t cur_key);
		uint32_t estimate(tuple_key_t cur_key);
		set<tuple_key_t> get_tuplekeys();
		uint32_t get_transmission_pkts();
		uint32_t get_transmission_bytes();

		vector< std::pair<tuple_key_t, uint32_t> > get_heap();
	private:
		uint32_t height;
		uint32_t width;
		uint32_t threshold;
		uint32_t heapsize;

		vector<uint32_t> buckets;
		vector< std::pair<tuple_key_t, uint32_t> > heap_vector;
		//map<tuple_key_t, uint32_t> heap_map;

		uint32_t* get_bucket(uint32_t row, uint32_t col);
};

#endif
