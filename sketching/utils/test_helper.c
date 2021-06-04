#include <algorithm>
#include <stdlib.h>
#include <math.h>

#include "./test_helper.h"

#include "../metric/cover_proportion.h"
#include "../metric/flowsize_metric.h"
#include "../metric/heavy_metric.h"
#include "../metric/other_metric.h"

void test() {
	// Update sketches
	update_sketch(sketch, tuplekey_vector);
	printf("Finish update\n");

	// Estimate Time
	set<tuple_key_t> tuplekey_set(tuplekey_vector.begin(), tuplekey_vector.end());
	decoding_test(sketch, tuplekey_set);
	transmission_test(sketch, tuplekey_vector, bytesum);

	// Get estimators
	Estimator estimator = get_estimator(sketch, tuplekey_vector);	

	// Flow size distribution
	flowsize_test(estimator, label);
	accuracy_test(estimator);

	// Cover proportion, hh detection, hc detection
	if (next_sketch != NULL) {
		update_sketch(next_sketch, next_tuplekey_vector);
		Estimator next_estimator = get_estimator(next_sketch, next_tuplekey_vector);	
		heavykey_test(estimator, next_estimator);
		heavykey_test(estimator, next_estimator, sketch, next_sketch);
	}

	// Pktdrop detection
	if (pktdrop_sketch != NULL) {
		update_sketch(pktdrop_sketch, pktdrop_tuplekey_vector); // Node B
		Estimator pktdrop_estimator = get_estimator(pktdrop_sketch, pktdrop_tuplekey_vector);
		pktdrop_test(estimator, pktdrop_estimator, dropped_tuplekeys); // Node A - Node B
	}

	// Incorrect routing detection
	if (incorrect_routing_sketch != NULL) {
		update_sketch(incorrect_routing_sketch, incorrect_routing_tuplekey_vector); // Node C
		Estimator incorrect_routing_estimator = get_estimator(incorrect_routing_sketch, incorrect_routing_tuplekey_vector);
		incorrect_routing_test(incorrect_routing_estimator, incorrect_routing_tuplekeys); // Node C
	}
}

vector<tuple_key_t> generate_debug_dataset() {
	// Tuple record for simple test
	vector<tuple_key_t> tuplekey_vector;
	uint32_t tmpnum = 16;
	tuple_key_t tmp_tuplekey;
	tmp_tuplekey.src_ip = 1;
	tmp_tuplekey.dst_ip = 2;
	for (uint32_t i = 0; i < tmpnum; i++) {
		tuplekey_vector.push_back(tmp_tuplekey);
	}
	tmpnum = 64;
	tmp_tuplekey.src_ip = 3;
	tmp_tuplekey.dst_ip = 4;
	for (uint32_t i = 0; i < tmpnum; i++) {
		tuplekey_vector.push_back(tmp_tuplekey);
	}
	return tuplekey_vector;
}

void debug(BaseSketch* sketch) {
	vector<tuple_key_t> tuplekey_vector = generate_debug_dataset();
	for (uint32_t i = 0; i < tuplekey_vector.size(); i++) {
		sketch->update(tuplekey_vector[i]);
	}

	set<tuple_key_t> tuplekey_set;
	tuplekey_set.insert(tuplekey_vector.begin(), tuplekey_vector.end());
	for (set<tuple_key_t>::iterator iter = tuplekey_set.begin(); iter != tuplekey_set.end(); iter++) {
		uint32_t result = sketch->estimate(*iter);
		LOG_MSG("srcip: %d, dstip: %d, estimate: %d\n", iter->src_ip, iter->dst_ip, result);
	}

	LOG_MSG("Tracked flowkeys...\n");
	tuplekey_set = sketch->get_tuplekeys();
	for (set<tuple_key_t>::iterator iter = tuplekey_set.begin(); iter != tuplekey_set.end(); iter++) {
		uint32_t result = sketch->estimate(*iter);
		LOG_MSG("srcip: %d, dstip: %d, estimate: %d\n", iter->src_ip, iter->dst_ip, result);
	}
}

vector<tuple_key_t> generate_synthetic_dataset(set<tuple_key_t>& tuplekey_set) {
	uint32_t x = 1;
	double total_p = 0.0;
	while (true) {
		double tmp_p = INTERCEPT*pow(double(x), -SLOPE);
		if (tmp_p < 1.0/FLOWNUM) {
			LOG_MSG("Theoretical max flow size: %u\n", x-1);
			break;
		}
		total_p += tmp_p;
		x += 1;
	}
	double ratio = 1.0/total_p;
	//LOG_MSG("Ratio: %lf\n", ratio);

	vector<tuple_key_t> result;
	srand(0);
	uint32_t flownum = 0;
	uint32_t pktnum = 0;
	x = 1;
	while (true) {
		if (flownum >= FLOWNUM) break;

		uint32_t y = uint32_t(ratio*FLOWNUM*INTERCEPT*pow(double(x), -SLOPE));
		if (y == 0) y=1;

		flownum += y;
		pktnum += x*y;
		for (uint32_t j = 0; j < y; j++) {
			tuple_key_t tmp_tuplekey;
			tmp_tuplekey.src_ip = rand();
			tmp_tuplekey.dst_ip = rand();
			tuplekey_set.insert(tmp_tuplekey);
			vector<tuple_key_t> tmp_vector(x, tmp_tuplekey);
			result.insert(result.end(), tmp_vector.begin(), tmp_vector.end());
		}
		x += 1;
	}
	LOG_MSG("Practical max flow size: %u\n", x-1);
	LOG_MSG("Flownum: %u, pktnum: %u, vector size: %u\n", flownum, pktnum, result.size());
	return result;
}

void update_sketch(BaseSketch* sketch, vector<tuple_key_t> tuplekey_vector) {
	LOG_MSG("Update sketch...\n");
	uint32_t pkt_cnt = tuplekey_vector.size();
	for (uint32_t i = 0; i < pkt_cnt; i++) {
		sketch->update(tuplekey_vector[i]);

		/*uint32_t tmp_est = sketch->estimate(tuplekey_vector[i]);*/
		/*LOG_MSG("Truth %d Est %d\n", i+1, tmp_est);*/
	}
}

void decoding_test(BaseSketch* sketch, set<tuple_key_t> tuplekey_set) {
	LOG_MSG("Decoding...\n");
	// set<tuple_key_t> tracked_tuplekeys = sketch->get_tuplekeys();
	clock_t start = clock();
	for (set<tuple_key_t>::iterator iter = tuplekey_set.begin(); iter != tuplekey_set.end(); iter++) { // One-pass
		sketch->estimate(*iter);
	}
	clock_t end = clock();
	double time_cost = double(end - start) * 1000 / double(CLOCKS_PER_SEC); // ms
	LOG_MSG("Estimate time: %f ms\n", time_cost);
}

void transmission_test(BaseSketch* sketch, vector<tuple_key_t> tuplekey_vector, uint32_t bytesum) {
	uint32_t pktsum = tuplekey_vector.size();

	uint32_t transmission_pkts = sketch->get_transmission_pkts();
	uint32_t transmission_bytes = sketch->get_transmission_bytes();
	LOG_MSG("Pkt ratio: %f, Byte ratio: %f\n", transmission_pkts/float(pktsum), transmission_bytes/float(bytesum));
}

void thpt_test(BaseSketch* sketch, vector<tuple_key_t> tuplekey_vector) {
	uint32_t nloop = 10;

	LOG_MSG("Update sketch...\n");
	uint32_t pkt_cnt = tuplekey_vector.size();
	clock_t start = clock();
	for (uint32_t loop_idx = 0; loop_idx < nloop; loop_idx++) {
		for (uint32_t i = 0; i < pkt_cnt; i++) {
			sketch->update(tuplekey_vector[i]);
		}
	}
	clock_t end = clock();

	double time_cost = double(end - start) * 1000 / double(CLOCKS_PER_SEC) / double(nloop); // ms per loop
	double throughput = 1000 * double(pkt_cnt) / double(time_cost) / 1024.0 / 1024.0;
	LOG_MSG("Loop: %d, packet cnt per loop: %d, time cost: %2f ms, throughput: %2f Mpps\n", \
			nloop, pkt_cnt, time_cost, throughput);
}

Estimator get_estimator(BaseSketch* sketch, vector<tuple_key_t> tuplekey_vector) {
	Estimator estimator = Estimator(tuplekey_vector);

	set<tuple_key_t> tracked_tuplekeys = sketch->get_tuplekeys();
	for (set<tuple_key_t>::iterator iter = tracked_tuplekeys.begin(); iter != tracked_tuplekeys.end(); iter++) { // One-pass
		uint32_t tmp_est = sketch->estimate(*iter);
		estimator.eval(*iter, tmp_est);
	}
	return estimator;
}

void flowsize_test(const Estimator& estimator, const char* label) {
	LOG_MSG("Dump flowsize metric...\n");
	char groundtruth_filename[256];
	char estimation_filename[256];
	sprintf(groundtruth_filename, "./tmp/%s_groundtruth_flowsize.bin", label);
	sprintf(estimation_filename, "./tmp/%s_estimation_flowsize.bin", label);

	FlowsizeMetric groundtruth_flowsize_metric = FlowsizeMetric(estimator.get_groundtruth());
	groundtruth_flowsize_metric.dump(groundtruth_filename);
	FlowsizeMetric estimation_flowsize_metric = FlowsizeMetric(estimator.get_estimation());
	estimation_flowsize_metric.dump(estimation_filename);
	float WMRD = groundtruth_flowsize_metric.get_WMRD(estimation_flowsize_metric);
	LOG_MSG("WMRD: %f\n", WMRD);
}

void accuracy_test(const Estimator& estimator) {
	CoverProportion cp = CoverProportion(estimator);
	cp.dump();

	dump_FPR_FNR(estimator);
	dump_onesize_flow(estimator);
	dump_cardinality_re(estimator);
	dump_entropy_re(estimator);
}

void heavykey_test(const Estimator& estimator, const Estimator& other) {
	HeavyMetric heavy_metric = HeavyMetric();
	heavy_metric.dump_hh_detection(estimator);
	heavy_metric.dump_hc_detection(estimator, other);
}

void heavykey_test(const Estimator& estimator, const Estimator& other, BaseSketch* sketch, BaseSketch* next_sketch) {
	HeavyMetric heavy_metric = HeavyMetric();
	heavy_metric.dump_hc_detection(estimator, other, sketch, next_sketch);
}

// Anomaly detection

vector<tuple_key_t> generate_pktdrop_dataset(vector<tuple_key_t> tuplekey_vector, set<tuple_key_t>& dropped_tuplekeys, float drop_ratio, bool need_remain) {
	map<tuple_key_t, uint32_t> tmp_map;
	for (uint32_t i = 0; i < tuplekey_vector.size(); i++) {
		if (tmp_map.find(tuplekey_vector[i]) != tmp_map.end()) {
			tmp_map[tuplekey_vector[i]] += 1;
		}
		else {
			tmp_map.insert(std::pair<tuple_key_t, uint32_t>(tuplekey_vector[i], 1));
		}
	}

	srand(0);
	uint32_t drop_value = drop_ratio * 100;
	for (map<tuple_key_t, uint32_t>::iterator iter = tmp_map.begin(); iter != tmp_map.end(); iter++) {
		uint32_t tmp = uint32_t(rand()) % 100;
		if (tmp < drop_value) {
			dropped_tuplekeys.insert(iter->first);
		}
	}

	vector<tuple_key_t> remain_result;
	vector<tuple_key_t> drop_result;
	for (uint32_t i = 0; i < tuplekey_vector.size(); i++) {
		if (dropped_tuplekeys.find(tuplekey_vector[i]) == dropped_tuplekeys.end()) {
			remain_result.push_back(tuplekey_vector[i]);
		}
		else {
			drop_result.push_back(tuplekey_vector[i]);
		}
	}

	if (need_remain) {
		return remain_result;
	}
	else {
		return drop_result;
	}
}

void pktdrop_test(const Estimator& estimator, const Estimator& other, set<tuple_key_t> dropped_tuplekeys) {
	HeavyMetric heavy_metric(1, 1);
	heavy_metric.dump_pktdrop_detection(estimator, other, dropped_tuplekeys);
}

void incorrect_routing_test(const Estimator& estimator, set<tuple_key_t> incorrect_routing_tuplekeys) {
	HeavyMetric heavy_metric(1, 1);
	heavy_metric.dump_incorrect_routing_detection(estimator, incorrect_routing_tuplekeys);
}
