#ifndef LOSSY_COUNTING_H
#define LOSSY_COUNTING_H

#include <stdint.h>
#include <map>
#include <vector>
#include <set>

#include "../tuple/tuple.h"
#include "./base_sketch.h"

using namespace std;

class LossyCounting: public BaseSketch {
	public:
		LossyCounting(uint32_t total_bytes, float epsilon);

		void update(tuple_key_t cur_key);
		uint32_t estimate(tuple_key_t cur_key);
		set<tuple_key_t> get_tuplekeys();
		uint32_t get_transmission_pkts();
		uint32_t get_transmission_bytes();
	private:
		float epsilon = 0.0;
		uint32_t cur_packetnum = 0;
		uint32_t prev_bucketidx = 0;

		uint32_t max_datasize = 0;
		//map<tuple_key_t, std::pair<uint32_t, uint32_t>> data;
		vector<tuple_key_t> tuplekeys;
		vector<uint32_t> frequencies;
		vector<uint32_t> bucketidxes;
};

#endif
