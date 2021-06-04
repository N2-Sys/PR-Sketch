#include <iostream>
#include <string>
#include <vector>
#include <set>

#include "../tuple/tuple.h"
#include "../tuple/tuple_records.h"
#include "../algorithm/lossy_counting.h"
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
	float unit_epsilon = 0.001; // For 1 MB
	float epsilon = unit_epsilon / (total_bytes / 1024.0 / 1024.0);

	// 1. Simple test for debugging
	if (DEBUG) {
		total_bytes = 1 * (8+4+4);
		sketch = new LossyCounting(total_bytes, epsilon);
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
	//next_bytesum = next_tuple_records.get_bytesum();
	sketch = new LossyCounting(total_bytes, epsilon);
	next_sketch = new LossyCounting(total_bytes, epsilon);

	LOG_MSG("\nTest with CAIDA...\n");
	test();
	delete sketch;
	sketch = new LossyCounting(total_bytes, epsilon); // new sketch
	thpt_test(sketch, tuplekey_vector);
	delete sketch;
	delete next_sketch;

	if (IS_SYNTHETIC) {
		next_sketch = NULL;
		pktdrop_sketch = NULL;
		sprintf(label, "synthetic");
		LOG_MSG("\nTest with synthetic dataset...\n");
		set<tuple_key_t> synthetic_tuplekeys;
		tuplekey_vector = generate_synthetic_dataset(synthetic_tuplekeys);
		bytesum = tuplekey_vector.size() * 64;
		sketch = new LossyCounting(total_bytes, epsilon);
		test();
		return 0;
	}
}
