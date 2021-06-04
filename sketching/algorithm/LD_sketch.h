#ifndef LD_SKETCH_H
#define LD_SKETCH_H

#include <stdint.h>
#include <vector>
#include <set>
#include <map>

#include "../tuple/tuple.h"
#include "./base_sketch.h"

using namespace std;

class LDSketchBucket {
	public:
		uint32_t V = 0; // total value
		uint32_t e = 0; // deviation error
		uint32_t length = 0; // length of array
		map<tuple_key_t, uint32_t> array; // <flowkey, single value>

		uint32_t bucket_update(tuple_key_t cur_key, uint32_t cur_nbucket);
		vector<tuple_key_t> get_bucket_tuplekeys();
		uint32_t bucket_estimate_lowerbound(tuple_key_t cur_key);
		uint32_t bucket_estimate(tuple_key_t cur_key);
};

class LDSketch: public BaseSketch {
	public:
		static uint32_t max_nbucket; // max dynamic bucket num
		static uint32_t max_length; // max length
		static uint32_t T; // threshold

		LDSketch(uint32_t total_bytes, uint32_t hashnum, uint32_t threshold, float ratio);

		void update(tuple_key_t cur_key);
		uint32_t estimate(tuple_key_t cur_key);
		uint32_t estimate_lowerbound(tuple_key_t cur_key);
		set<tuple_key_t> get_tuplekeys();
		uint32_t get_transmission_pkts();
		uint32_t get_transmission_bytes();

		set<tuple_key_t> get_hcs(BaseSketch* other, uint32_t threshold);
	private:
		uint32_t height = 0;
		uint32_t width = 0;
		uint32_t cur_nbucket = 0; // current dynamic bucket num

		vector<LDSketchBucket> buckets;

		LDSketchBucket* get_bucket(uint32_t row, uint32_t col);
};

#endif
