		#ifndef CM_SKETCH_H
#define CM_SKETCH_H

#include <stdint.h>
#include <vector>
#include <set>

#include "../tuple/tuple.h"
#include "./base_sketch.h"

using namespace std;

class CMSketch: public BaseSketch {
	public:
		CMSketch(uint32_t total_bytes, uint32_t height);

		void update(tuple_key_t cur_key);
		uint32_t estimate(tuple_key_t cur_key);
		set<tuple_key_t> get_tuplekeys();
		uint32_t get_transmission_pkts();
		uint32_t get_transmission_bytes();
	private:
		uint32_t height;
		uint32_t width;

		vector<uint32_t> buckets;

		uint32_t* get_bucket(uint32_t row, uint32_t col);
};

#endif
