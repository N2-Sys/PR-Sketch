#include <iostream>
#include <string>
#include <vector>
#include <set>

#include "../tuple/tuple.h"
#include "../tuple/tuple_records.h"
#include "../algorithm/MRAC.h"
#include "../utils/util.h"
#include "../utils/estimator.h"
#include "../utils/test_helper.h"

#include "../metric/flowsize_metric.h"

using namespace std;

char label[256];

vector<tuple_key_t> tuplekey_vector;
uint32_t bytesum = 0;
BaseSketch* sketch = NULL;

vector<tuple_key_t> next_tuplekey_vector;
uint32_t next_bytesum = 0;
BaseSketch* next_sketch = NULL;

vector<tuple_key_t> pktdrop_tuplekey_vector;
BaseSketch* pktdrop_sketch = NULL;
set<tuple_key_t> dropped_tuplekeys;

vector<tuple_key_t> incorrect_routing_tuplekey_vector;
BaseSketch* incorrect_routing_sketch = NULL;
set<tuple_key_t> incorrect_routing_tuplekeys;

void MRAC_flowsize_test(MRAC* sketch, vector<tuple_key_t> tuplekey_vector, char* label) {
	update_sketch(sketch, tuplekey_vector);

	LOG_MSG("Dump flowsize metric...\n");
	char groundtruth_filename[256];
	char estimation_filename[256];
	sprintf(groundtruth_filename, "./tmp/%s_groundtruth_flowsize.bin", label);
	sprintf(estimation_filename, "./tmp/%s_estimation_flowsize.bin", label);

	FlowsizeMetric groundtruth_flowsize_metric = FlowsizeMetric(tuplekey_vector);
	groundtruth_flowsize_metric.dump(groundtruth_filename);
	map<uint32_t, uint32_t> estimation_flowsize_map = sketch->get_flowsize_map();
	FlowsizeMetric estimation_flowsize_metric = FlowsizeMetric(estimation_flowsize_map);
	estimation_flowsize_metric.dump(estimation_filename);
	float WMRD = groundtruth_flowsize_metric.get_WMRD(estimation_flowsize_metric);
	LOG_MSG("WMRD: %f\n", WMRD);
}

int main(int argc, char* argv[]){
	uint32_t total_bytes = 0;
	if (argc != 2) {
		LOG_ERR("Invalid format which should be %s total_memory(KB)\n", argv[0]);
	}
	else {
		total_bytes = stoi(string(argv[1])) * 1024;
	}

	// 1. Simple test for debugging
	if (DEBUG) {
		total_bytes = 1 * 4;
		sketch = new MRAC(total_bytes);
		debug(sketch);
		delete sketch;
		exit(0);
	}

	// 2. Test with CAIDA
	memset(label, '\0', 256);
	sprintf(label, "normal");

	// Load normal tuples
	TupleRecords tuple_records(TRACE_PATH);
	tuplekey_vector = tuple_records.read_all_tuplekey();
	bytesum = tuple_records.get_bytesum();
	MRAC* mysketch = new MRAC(total_bytes);

	LOG_MSG("\nTest with CAIDA...\n");
	// Flow size distribution
	MRAC_flowsize_test(mysketch, tuplekey_vector, label);
	delete mysketch;
}
