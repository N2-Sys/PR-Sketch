#ifndef ELASTIC_SKETCH_H
#define ELASTIC_SKETCH_H

#include <stdint.h>
#include <vector>
#include <set>

#include "../tuple/tuple.h"
#include "./base_sketch.h"

using namespace std;

class ElasticSketchHeavyCounter{
	public:
		vector<uint32_t> pvotes;
		vector<tuple_key_t> tuplekeys;
		vector<bool> flags;

		uint32_t nvote;

		ElasticSketchHeavyCounter(uint32_t len);
};

class ElasticSketch: public BaseSketch {
	public:
		ElasticSketch(uint32_t filter_space, uint32_t measure_space, uint32_t counter_len, uint32_t height);

		void update(tuple_key_t cur_key);
		uint32_t estimate(tuple_key_t cur_key);
		set<tuple_key_t> get_tuplekeys();
		uint32_t get_transmission_pkts();
		uint32_t get_transmission_bytes();
	private:
		uint32_t counter_len;
		uint32_t heavy_bucketnum;
		uint32_t height;
		uint32_t width;

		vector<ElasticSketchHeavyCounter> heavy_counter_vector;
		vector<uint32_t> light_vector;

		uint32_t* get_light_count(uint32_t row, uint32_t col);
};

#endif
