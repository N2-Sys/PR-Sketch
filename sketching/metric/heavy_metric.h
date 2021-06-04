#ifndef HEAVY_METRIC_H
#define HEAVY_METRIC_H

#include <set>
#include <map>
#include <stdint.h>

#include "../tuple/tuple.h"
#include "../utils/util.h"
#include "../utils/estimator.h"
#include "../algorithm/base_sketch.h"

class HeavyMetric {
	public:
		HeavyMetric(uint32_t hh_threshold = HH_THRESHOLD, uint32_t hc_threshold = HC_THRESHOLD);

		void dump_hh_detection(const Estimator& estimator);
		void dump_hc_detection(const Estimator& estimator, const Estimator& other);
		void dump_hc_detection(const Estimator& estimator, const Estimator& other, BaseSketch* sketch, BaseSketch* next_sketch);
		void dump_pktdrop_detection(const Estimator& estimator, const Estimator& other, set<tuple_key_t> dropped_tuplekeys);
		void dump_incorrect_routing_detection(const Estimator& estimator, set<tuple_key_t> incorrect_routing_tuplekeys);
	private:
		uint32_t hh_threshold = 0;
		uint32_t hc_threshold = 0;
		set<tuple_key_t> get_hhs(const map<tuple_key_t, uint32_t>& curmap, uint32_t threshold);
		set<tuple_key_t> get_hcs(const map<tuple_key_t, uint32_t>& prevmap, const map<tuple_key_t, uint32_t>& nextmap, uint32_t threshold);
		void dump_precision_recall(set<tuple_key_t> groundtruth_set, set<tuple_key_t> estimation_set); 
};


#endif
