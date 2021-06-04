#ifndef ESTIMATOR_H
#define ESTIMATOR_H

#include <iostream>
#include <map>
#include <vector>
#include <set>

#include "../tuple/tuple.h"
#include "./util.h"

using namespace std;

class Estimator{
	public:
		Estimator(vector<tuple_key_t> tuple_vector);

		// Eval
		void eval(tuple_key_t cur_tuple, uint32_t estimation_value);

		// After eval
		const map<tuple_key_t, uint32_t>& get_groundtruth() const;
		const set<uint32_t>& get_srcip_set() const;
		const set<uint32_t>& get_dstip_set() const;
		const set<tuple_key_t>& get_truth_tuplekey_set() const;
		const map<tuple_key_t, uint32_t>& get_estimation() const;
		const set<tuple_key_t>& get_predict_tuplekey_set() const;
		uint32_t get_keyspace() const;
	private:
		// Before eval
		map<tuple_key_t, uint32_t> groundtruth;
		set<uint32_t> srcip_set;
		set<uint32_t> dstip_set;
		set<tuple_key_t> truth_tuplekey_set;

		// After eval
		map<tuple_key_t, uint32_t> estimation;
		set<tuple_key_t> predict_tuplekey_set;
};

#endif
