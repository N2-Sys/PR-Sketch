#ifndef DELTOID_H
#define DELTOID_H

#include <stdint.h>
#include <vector>
#include <set>
#include <map>

#include "../tuple/tuple.h"
#include "./base_sketch.h"

using namespace std;

class Deltoid: public BaseSketch {
	public:
		Deltoid(uint32_t total_bytes, uint32_t height, uint32_t keylen, uint32_t threshold);

		void update(tuple_key_t cur_key);
		uint32_t estimate(tuple_key_t cur_key);
		set<tuple_key_t> get_tuplekeys();
		uint32_t get_transmission_pkts();
		uint32_t get_transmission_bytes();
	private:
		uint32_t height;
		uint32_t width;
		uint32_t keylen;
		uint32_t threshold;

		vector<vector<uint32_t>> groups;

		vector<uint32_t>* get_group(uint32_t row, uint32_t col);

		bool is_decode = false;
		set<tuple_key_t> decode_tuplekeys;
		map<tuple_key_t, uint32_t> decode_result;
		void decode();
};

#endif
