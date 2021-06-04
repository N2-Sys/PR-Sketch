#include "./cover_proportion.h"

CoverProportion::CoverProportion(const Estimator& estimator) {
	const map<tuple_key_t, uint32_t>& estimation = estimator.get_estimation();
	const map<tuple_key_t, uint32_t>& groundtruth = estimator.get_groundtruth();
	for (map<tuple_key_t, uint32_t>::const_iterator iter=estimation.begin(); iter!=estimation.end(); iter++) {
		if (estimation_error.find(iter->first) == estimation_error.end()) {
			float tmp_error = 1.0;
			if (groundtruth.find(iter->first) != groundtruth.end()) {
				uint32_t truth_value = groundtruth.at(iter->first);
				tmp_error = float(abs_minus(truth_value, iter->second)) / float(truth_value);
			}
			estimation_error.insert(std::pair<tuple_key_t, float>(iter->first, tmp_error));
		}
	}
	groundtruth_size = groundtruth.size();
}

void CoverProportion::dump() {
	uint32_t count_minimal = 0;
	uint32_t count_10 = 0;
	uint32_t count_50 = 0;
	for (map<tuple_key_t, float>::const_iterator iter = estimation_error.begin(); iter != estimation_error.end(); iter++) {
		if (iter->second < 0.001) {
			count_minimal += 1;
		}
		if (iter->second < 0.1) {
			count_10 += 1;
		}
		if (iter->second < 0.5) {
			count_50 += 1;
		}
	}

	float ratio_minimal = float(count_minimal)/float(groundtruth_size);
	float ratio_10 = float(count_10)/float(groundtruth_size);
	float ratio_50 = float(count_50)/float(groundtruth_size);
	LOG_MSG("Ratio (RE < 0.1%): %f\n", ratio_minimal);
	LOG_MSG("Ratio (RE < 10%): %f\n", ratio_10);
	LOG_MSG("Ratio (RE < 50%): %f\n", ratio_50);
}
