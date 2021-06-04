#include <stdlib.h>
#include <algorithm>

#include "./estimator.h"

Estimator::Estimator(vector<tuple_key_t> tuple_vector) {
	for (uint32_t i = 0; i < tuple_vector.size(); i++) {
		tuple_key_t tmp_tuplekey = tuple_vector[i];
		if (groundtruth.find(tmp_tuplekey) == groundtruth.end()) {
			groundtruth.insert(std::pair<tuple_key_t, uint32_t>(tmp_tuplekey, 1));
		}
		else {
			groundtruth[tmp_tuplekey] += 1;
		}

		srcip_set.insert(tmp_tuplekey.src_ip);
		dstip_set.insert(tmp_tuplekey.dst_ip);
		truth_tuplekey_set.insert(tmp_tuplekey);
	}
}

void Estimator::eval(tuple_key_t cur_tuple, uint32_t estimation_value) {
	if (estimation.find(cur_tuple) == estimation.end()) {
		estimation.insert(std::pair<tuple_key_t, uint32_t>(cur_tuple, estimation_value));
		predict_tuplekey_set.insert(cur_tuple);
	}
}

const map<tuple_key_t, uint32_t>& Estimator::get_groundtruth() const {
	return groundtruth;
}

const set<uint32_t>& Estimator::get_srcip_set() const {
	return srcip_set;
}

const set<uint32_t>& Estimator::get_dstip_set() const {
	return dstip_set;
}

const set<tuple_key_t>& Estimator::get_truth_tuplekey_set() const {
	return truth_tuplekey_set;
}

const map<tuple_key_t, uint32_t>& Estimator::get_estimation() const {
	return estimation;
}

const set<tuple_key_t>& Estimator::get_predict_tuplekey_set() const {
	return predict_tuplekey_set;
}

uint32_t Estimator::get_keyspace() const {
	uint32_t keyspace = srcip_set.size() * dstip_set.size();
	return keyspace;
}
