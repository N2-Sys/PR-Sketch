#ifndef MV_SKETCH_H
#define MV_SKETCH_H

#include <stdint.h>
#include <vector>
#include <set>

#include "../tuple/tuple.h"
#include "./base_sketch.h"

using namespace std;

typedef struct MV_sketch_bucket {
	uint32_t V; // total value
	tuple_key_t tuplekey;
	int32_t C; // single value
} MVSketchBucket;

class MVSketch: public BaseSketch {
	public:
		MVSketch(uint32_t total_bytes, uint32_t height);

		void update(tuple_key_t cur_key);
		uint32_t estimate(tuple_key_t cur_key);
		set<tuple_key_t> get_tuplekeys();
		uint32_t get_transmission_pkts();
		uint32_t get_transmission_bytes();
	private:
		uint32_t height;
		uint32_t width;

		vector<MVSketchBucket> buckets;

		MVSketchBucket* get_bucket(uint32_t row, uint32_t col);
};

#endif
