#include <iostream>
#include <string>
#include <vector>
#include <set>

#include "../tuple/tuple.h"
#include "../tuple/tuple_records.h"
#include "../algorithm/pr_sketch.h"
#include "../utils/util.h"
#include "../utils/estimator.h"
#include "../utils/test_helper.h"

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
	uint32_t filter_memory = 0;
	uint32_t measure_memory = 0;
	if (argc != 3) {
		LOG_ERR("Invalid format which should be %s filter_memory(KB) measure_memory(KB)\n", argv[0]);
	}
	else {
		filter_memory = stoi(string(argv[1])) * 1024;
		measure_memory = stoi(string(argv[2])) * 1024;
	}
	uint32_t bf_space = filter_memory / 4;
	uint32_t measure_space = measure_memory / 4;
	uint32_t bf_hash_num = 1;
	uint32_t cbf_hash_num = 1;

	// 1. Simple test for debugging
	if (DEBUG) {
		sketch = new BFCBF(bf_space, measure_space, bf_hash_num, cbf_hash_num);
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
	sketch = new BFCBF(bf_space, measure_space, bf_hash_num, cbf_hash_num);
	next_sketch = new BFCBF(bf_space, measure_space, bf_hash_num, cbf_hash_num);

	pktdrop_sketch = new BFCBF(bf_space, measure_space, bf_hash_num, cbf_hash_num);
	pktdrop_tuplekey_vector = generate_pktdrop_dataset(tuplekey_vector, dropped_tuplekeys, 0.5);

	LOG_MSG("\nTest with CAIDA...\n");
	test();
	LOG_MSG("Key recording memory cost: %d KB\n", sketch->get_tuplekeys().size()*8/1024);
	deleteptr(sketch);
	deleteptr(next_sketch);
	deleteptr(pktdrop_sketch);
	//sketch = new BFCBF(bf_space, measure_space, bf_hash_num, cbf_hash_num);
	//thpt_test(sketch, tuplekey_vector);
	//delete sketch;
	//delete next_sketch;

	if (IS_SYNTHETIC) {
		next_sketch = NULL;
		pktdrop_sketch = NULL;
		sprintf(label, "synthetic");
		LOG_MSG("\nTest with synthetic dataset...\n");
		set<tuple_key_t> synthetic_tuplekeys;
		tuplekey_vector = generate_synthetic_dataset(synthetic_tuplekeys);
		bytesum = tuplekey_vector.size() * 64;
		sketch = new BFCBF(bf_space, measure_space, bf_hash_num, cbf_hash_num);
		test();
		return 0;
	}
}
