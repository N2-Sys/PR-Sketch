#include "./heavy_metric.h"

HeavyMetric::HeavyMetric(uint32_t hh_threshold, uint32_t hc_threshold) {
	this->hh_threshold = hh_threshold;
	this->hc_threshold = hc_threshold;
}

void HeavyMetric::dump_hh_detection(const Estimator& estimator) {
	const map<tuple_key_t, uint32_t>& groundtruth = estimator.get_groundtruth();
	const map<tuple_key_t, uint32_t>& estimation = estimator.get_estimation();

	set<tuple_key_t> groundtruth_hhs = get_hhs(groundtruth, hh_threshold);
	set<tuple_key_t> estimation_hhs = get_hhs(estimation, hh_threshold);
	LOG_MSG("[heavy hitter detection]:\n");
	dump_precision_recall(groundtruth_hhs, estimation_hhs);
}

void HeavyMetric::dump_hc_detection(const Estimator& estimator, const Estimator& other) {
	set<tuple_key_t> groundtruth_hcs = get_hcs(estimator.get_groundtruth(), other.get_groundtruth(), hc_threshold);
	set<tuple_key_t> estimation_hcs = get_hcs(estimator.get_estimation(), other.get_estimation(), hc_threshold);
	LOG_MSG("[heavy changer detection]:\n");
	dump_precision_recall(groundtruth_hcs, estimation_hcs);
}

void HeavyMetric::dump_hc_detection(const Estimator& estimator, const Estimator& other, BaseSketch* sketch, BaseSketch* next_sketch) {
	afs_assert(sketch!=NULL, "NULL pointer of sketch in dump_hc_detection!\n");
	afs_assert(next_sketch!=NULL, "NULL pointer of next_sketch in dump_hc_detection!\n");
	set<tuple_key_t> groundtruth_hcs = get_hcs(estimator.get_groundtruth(), other.get_groundtruth(), hc_threshold);
	set<tuple_key_t> estimation_hcs = sketch->get_hcs(next_sketch, hc_threshold);
	LOG_MSG("[heavy changer detection based on sketch itself]:\n");
	dump_precision_recall(groundtruth_hcs, estimation_hcs);
}

void HeavyMetric::dump_pktdrop_detection(const Estimator& estimator, const Estimator& other, set<tuple_key_t> dropped_tuplekeys) {
	set<tuple_key_t> estimation_drops = get_hcs(estimator.get_estimation(), other.get_estimation(), hc_threshold);
	LOG_MSG("[pktdrop detection]:\n");
	dump_precision_recall(dropped_tuplekeys, estimation_drops);
}

void HeavyMetric::dump_incorrect_routing_detection(const Estimator& estimator, set<tuple_key_t> incorrect_routing_tuplekeys) {
	const map<tuple_key_t, uint32_t>& estimation = estimator.get_estimation();
	set<tuple_key_t> estimation_hhs = get_hhs(estimation, hh_threshold);
	set<tuple_key_t> report_tuplekeys = estimation_hhs; // Report incorrect routing keys based on routing configuration
	LOG_MSG("[incorrect routing detection]:\n");
	dump_precision_recall(incorrect_routing_tuplekeys, report_tuplekeys);
}

set<tuple_key_t> HeavyMetric::get_hhs(const map<tuple_key_t, uint32_t>& curmap, uint32_t threshold) {
	set<tuple_key_t> result;
	for (map<tuple_key_t, uint32_t>::const_iterator iter = curmap.begin(); iter != curmap.end(); iter++) {
		if (iter->second >= threshold) {
			result.insert(iter->first);
		}
	}
	return result;
}

set<tuple_key_t> HeavyMetric::get_hcs(const map<tuple_key_t, uint32_t>& prevmap, const map<tuple_key_t, uint32_t>& nextmap, uint32_t threshold) {
	map<tuple_key_t, uint32_t> tmp = prevmap;
	for (map<tuple_key_t, uint32_t>::const_iterator iter = nextmap.begin(); iter != nextmap.end(); iter++) {
		if (tmp.find(iter->first) != tmp.end()) {
			tmp[iter->first] = abs_minus(iter->second, tmp[iter->first]);
		}
		else {
			tmp.insert(std::pair<tuple_key_t, uint32_t>(iter->first, iter->second));
		}
	}

	set<tuple_key_t> result;
	for (map<tuple_key_t, uint32_t>::iterator iter = tmp.begin(); iter != tmp.end(); iter++) {
		if (iter->second >= threshold) {
			result.insert(iter->first);
		}
	}
	return result;
}

void HeavyMetric::dump_precision_recall(set<tuple_key_t> groundtruth_set, set<tuple_key_t> estimation_set) {
	uint32_t TP = 0;
	for (set<tuple_key_t>::iterator iter = estimation_set.begin(); iter != estimation_set.end(); iter++) {
		if (groundtruth_set.find(*iter) != groundtruth_set.end()) {
			TP += 1;
		}
	}
	float precision = 0.0;
	if (estimation_set.size() != 0) {
		precision = float(TP)/estimation_set.size();
	}
	float recall = float(TP)/groundtruth_set.size();
	float F1 = 0.0;
	if (precision + recall > 0.0000000001) {
		F1 = 2*precision*recall/(precision+recall);
	}
	LOG_MSG("TP: %u, groundtruth: %lu, estimation: %lu\n", TP, groundtruth_set.size(), estimation_set.size());
	LOG_MSG("Precision: %f, Recall: %f, F1 score: %f\n", precision, recall, F1);
}


