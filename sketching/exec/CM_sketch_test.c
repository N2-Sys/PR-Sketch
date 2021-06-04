#include <iostream>
#include <string>
#include <vector>
#include <set>

#include "../tuple/tuple.h"
#include "../tuple/tuple_records.h"
#include "../algorithm/CM_sketch.h"
#include "../utils/util.h"
#include "../utils/estimator.h"
#include "../utils/test_helper.h"

#include "../utils/debug.h"

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

int main(int argc, char* argv[]){
	uint32_t total_bytes = 0;
	if (argc != 2) {
		LOG_ERR("Invalid format which should be %s total_memory(KB)\n", argv[0]);
	}
	else {
		total_bytes = stoi(string(argv[1])) * 1024;
	}
	uint32_t height = 2;

	// 1. Simple test for debugging
	if (DEBUG) {
		total_bytes = height * 1 * 4;
		sketch = new CMSketch(total_bytes, height);
		debug(sketch);
		delete sketch;
		exit(0);
	}

	// 2. Test with CAIDA
	memset(label, '\0', 256);
	sprintf(label, "normal");

	// Load normal tuples
	TupleRecords tuple_records(TRACE_PATH);
	TupleRecords next_tuple_records(NEXT_TRACE_PATH);
	tuplekey_vector = tuple_records.read_all_tuplekey();
	next_tuplekey_vector = next_tuple_records.read_all_tuplekey();
	bytesum = tuple_records.get_bytesum();
	LOG_MSG("Bytesum: %u\n", bytesum);
	//next_bytesum = next_tuple_records.get_bytesum();
	sketch = new CMSketch(total_bytes, height);
	next_sketch = new CMSketch(total_bytes, height);

	//DEBUG
	find_hh_threshold(tuplekey_vector);
	find_hc_threshold(tuplekey_vector, next_tuplekey_vector);
	count_assumption_violation(tuplekey_vector, height, total_bytes / 4 / height);
	return 0;

	LOG_MSG("\nTest with CAIDA...\n");
	test();
	delete sketch;
	sketch = new CMSketch(total_bytes, height); // new sketch
	thpt_test(sketch, tuplekey_vector);
	delete sketch;
	delete next_sketch;
}
