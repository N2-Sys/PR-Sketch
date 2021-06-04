#ifndef FLOWRADAR_H
#define FLOWRADAR_H

#include <stdint.h>
#include <vector>
#include <set>
#include <map>

#include "../tuple/tuple.h"
#include "./base_sketch.h"

using namespace std;

class FlowRadarCounter{
	public:
		uint32_t pktcnt;
		tuple_key_t tuplekey;
		uint32_t flowcnt;

		FlowRadarCounter();
		FlowRadarCounter(const FlowRadarCounter& other);
};

class FlowRadar: public BaseSketch {
	public:
		FlowRadar(uint32_t bf_space, uint32_t measure_space, uint32_t bf_hash_num, uint32_t cbf_hash_num);

		void update(tuple_key_t cur_key);
		uint32_t estimate(tuple_key_t cur_key);
		set<tuple_key_t> get_tuplekeys();
		uint32_t get_transmission_pkts();
		uint32_t get_transmission_bytes();
	private:
		uint32_t bf_hash_num;
		uint32_t bf_bucketnum;
		uint32_t cbf_hash_num;
		uint32_t cbf_bucketnum;

		vector<bool> bf_vector;
		vector<FlowRadarCounter> cbf_vector;

		bool is_decode = false;
		map<tuple_key_t, uint32_t> decode_result;
		void decode();
};

#endif
