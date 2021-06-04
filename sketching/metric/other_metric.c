#include <cmath>

#include "./other_metric.h"

void dump_FPR_FNR(const Estimator& estimator) {
	const set<tuple_key_t>& truth_tuplekey_set = estimator.get_truth_tuplekey_set();
   	const set<tuple_key_t>& predict_tuplekey_set = estimator.get_predict_tuplekey_set();
	uint32_t total_space = estimator.get_keyspace();

	// Compute FPR and FNR
	uint32_t TP = interset_cnt(truth_tuplekey_set, predict_tuplekey_set);
	uint32_t FN = truth_tuplekey_set.size() - TP;
	uint32_t FP = predict_tuplekey_set.size() - TP;
	uint32_t TN = total_space - truth_tuplekey_set.size() - FP;

	float FPR = float(FP) / float(FP + TN);
	float FNR = float(FN) / float(TP + FN);
	float precision = float(TP) / predict_tuplekey_set.size();
	float recall = float(TP) / truth_tuplekey_set.size();
	LOG_MSG("FPR: %f, FNR: %f, Precision: %f, Recall: %f\n", FPR, FNR, precision, recall);
}

void dump_onesize_flow(const Estimator& estimator) {
	const map<tuple_key_t, uint32_t>& groundtruth = estimator.get_groundtruth();
	const map<tuple_key_t, uint32_t>& estimation = estimator.get_estimation();

	uint32_t groundtruth_1size_nflow = 0;
	uint32_t predict_1size_nflow = 0;
	uint32_t hit_1size_nflow = 0;
	// uint32_t pktsum = 0;
	for (map<tuple_key_t, uint32_t>::const_iterator iter = groundtruth.begin(); iter != groundtruth.end(); iter++) {
		if (iter->second == 1) {
			groundtruth_1size_nflow += 1;
		}
		// pktsum += iter->second;
	}
	for (map<tuple_key_t, uint32_t>::const_iterator iter = estimation.begin(); iter != estimation.end(); iter++) {
		if (iter->second == 1) {
			predict_1size_nflow += 1;
			if (groundtruth.at(iter->first) == 1) {
				hit_1size_nflow += 1;
			}
		}
	}
	float precision = float(hit_1size_nflow) / predict_1size_nflow;
	float recall = float(hit_1size_nflow) / groundtruth_1size_nflow;
	float F1 = 2*precision*recall/(precision+recall);

	LOG_MSG("[1-size flow detection]:");
	// LOG_MSG("Pktsum: %d\n", pktsum);
	LOG_MSG("# of groundtruth: %d, # of estimation: %d, # of hit: %d\n", groundtruth_1size_nflow, predict_1size_nflow, hit_1size_nflow);
	LOG_MSG("Precision: %f, recall: %f, F1: %f\n", precision, recall, F1);
}

void dump_cardinality_re(const Estimator& estimator) {
	uint32_t groundtruth_size = estimator.get_groundtruth().size();
	uint32_t estimation_size = estimator.get_estimation().size();
	LOG_MSG("Groundtruth cardinality: %d, estimation cardinality: %d\n", groundtruth_size, estimation_size);
	uint32_t diff = abs_minus(groundtruth_size, estimation_size);
	float re = float(diff) / float(groundtruth_size);
	LOG_MSG("Cardinality relative error: %f\n", re);
}

void dump_entropy_re(const Estimator& estimator) {
	const map<tuple_key_t, uint32_t>& groundtruth = estimator.get_groundtruth();
	const map<tuple_key_t, uint32_t>& estimation = estimator.get_estimation();

	float sum = 0.0;
	for (map<tuple_key_t, uint32_t>::const_iterator iter = groundtruth.begin(); iter != groundtruth.end(); iter++) {
		sum += float(iter->second);
	}
	float groundtruth_entropy = 0.0;
	float probability = 0.0;
	for (map<tuple_key_t, uint32_t>::const_iterator iter = groundtruth.begin(); iter != groundtruth.end(); iter++) {
		probability = float(iter->second) / sum;
		if (probability == 0) {
			continue;
		}
		groundtruth_entropy += -probability * log(probability);
	}

	sum = 0.0;
	for (map<tuple_key_t, uint32_t>::const_iterator iter = estimation.begin(); iter != estimation.end(); iter++) {
		sum += float(iter->second);
	}
	float estimation_entropy = 0.0;
	for (map<tuple_key_t, uint32_t>::const_iterator iter = estimation.begin(); iter != estimation.end(); iter++) {
		probability = float(iter->second) / sum;
		if (probability == 0) {
			continue;
		}
		estimation_entropy += -probability * log(probability);
	}

	float re = abs(groundtruth_entropy - estimation_entropy) / groundtruth_entropy;
	LOG_MSG("Entropy relative error: %f\n", re);
}

uint32_t interset_cnt(const set<tuple_key_t>& a, const set<tuple_key_t>& b) {
	uint32_t result = 0;
	for (set<tuple_key_t>::const_iterator iter = a.begin(); iter != a.end(); iter++) {
		if (b.find(*iter) != b.end()) {
			result += 1;
		}
	}
	return result;
}
