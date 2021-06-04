#include <iostream>
#include <string>
#include <vector>
#include <set>

#include "../tuple/tuple.h"
#include "../tuple/tuple_records.h"
#include "../algorithm/deltoid.h"
#include "../utils/util.h"
#include "../utils/estimator.h"
#include "../utils/test_helper.h"

using namespace std;

char label[256];

vector<tuple_key_t> tuplekey_vector;
uint32_t bytesum = 0;
BaseSketch* sketch;

vector<tuple_key_t> next_tuplekey_vector;
uint32_t next_bytesum = 0;
BaseSketch* next_sketch;

vector<tuple_key_t> pktdrop_tuplekey_vector;
BaseSketch* pktdrop_sketch = NULL;
set<tuple_key_t> dropped_tuplekeys;

vector<tuple_key_t> incorrect_routing_tuplekey_vector;
BaseSketch* incorrect_routing_sketch = NULL;
set<tuple_key_t> incorrect_routing_tuplekeys;

int main(int argc, char* argv[]){
	uint32_t total_bytes = 0;
	uint32_t threshold = 0;
	if (argc != 3) {
		LOG_ERR("Invalid format which should be %s total_memory(KB)\n", argv[0]);
	}
	else {
		total_bytes = stoi(string(argv[1])) * 1024;
		threshold = stoi(string(argv[2]));
	}
	uint32_t height = 2;
	uint32_t keylen = sizeof(tuple_key_t) * BYTESIZE;
	// uint32_t threshold = HH_THRESHOLD * 4;
	// uint32_t threshold = 1; // For cover proportion

	// 1. Simple test for debugging
	if (DEBUG) {
		// total_bytes = height * 1 * (keylen + 1) * 4;
		sketch = new Deltoid(total_bytes, height, keylen, threshold);
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
	sketch = new Deltoid(total_bytes, height, keylen, threshold);
	next_sketch = new Deltoid(total_bytes, height, keylen, threshold);

	LOG_MSG("\nTest with CAIDA...\n");
	test();
	delete sketch;
	sketch = new Deltoid(total_bytes, height, keylen, threshold); // new sketch
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
		sketch = new Deltoid(total_bytes, height, keylen, threshold);
		test();
		return 0;
	}
}
