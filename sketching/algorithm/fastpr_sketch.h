#ifndef CBF_LEGITIMATE_BF_H
#define CBF_LEGITIMATE_BF_H

#include <stdint.h>
#include <vector>
#include <set>
#include <map>

#include "../tuple/tuple.h"
#include "./base_sketch.h"

using namespace std;

class CBFLegitimateBF: public BaseSketch {
	public:
		CBFLegitimateBF(uint32_t bf_space, uint32_t measure_space, uint32_t bf_hash_num, uint32_t cbf_hash_num, uint32_t threshold);

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
		uint32_t threshold;

		vector<bool> bf_vector;
		vector<uint32_t> cbf_vector;

		vector<tuple_key_t> bf_kick_tuplekey_vector;

		uint32_t traditional_estimate(tuple_key_t cur_key);

		bool is_solve = false;
		map<tuple_key_t, uint32_t> solution;
		void equation_solve();
};

#endif
