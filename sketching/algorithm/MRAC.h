#ifndef MRAC_H
#define MRAC_H

#include <stdint.h>
#include <vector>
#include <set>
#include <map>

#include "../tuple/tuple.h"
#include "./base_sketch.h"

using namespace std;

typedef vector<uint32_t> collision_type_t;
typedef vector<collision_type_t> collisions_type_t;

class CollisionGenerator {
	public:
		CollisionGenerator(uint32_t max_sum, uint32_t max_num=MRAC_MAXNUM);

		collisions_type_t get_all_collisions(uint32_t cur_sum);
	private:
		uint32_t max_sum = 0;
		uint32_t max_num = 0;
		vector<vector<collisions_type_t>> DP_data; // dynamic programming (max_num * max_sum)

		collisions_type_t* get_collisions(uint32_t cur_num, uint32_t cur_sum);
		bool check_collision(collision_type_t collision, uint32_t minv); // Check whether each element in coliision >= minv
		void append_collision(collision_type_t* cur_collision, collision_type_t other); // Append other into back of cur_collision
};

class MRAC: public BaseSketch {
	public:
		MRAC(uint32_t total_bytes);

		void update(tuple_key_t cur_key);
		uint32_t estimate(tuple_key_t cur_key);
		set<tuple_key_t> get_tuplekeys();
		uint32_t get_transmission_pkts();
		uint32_t get_transmission_bytes();

		map<uint32_t, uint32_t> get_flowsize_map();
	private:
		uint32_t width = 0;
		vector<uint32_t> buckets;

		bool is_EM = false;
		map<uint32_t, uint32_t> EM_result;
		void EM();

		double get_conditioned_probability(collision_type_t cur_collision, vector<double> phi, uint32_t n, uint32_t v, CollisionGenerator& collision_generator);
		double get_probability(collision_type_t cur_collision, vector<double> phi, uint32_t n);
		uint32_t get_nbucket(uint32_t value);
		uint32_t get_maxv();
};

#endif
