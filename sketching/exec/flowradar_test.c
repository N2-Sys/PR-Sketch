#include <iostream>
#include <string>
#include <vector>
#include <set>

#include "../tuple/tuple.h"
#include "../tuple/tuple_records.h"
#include "../algorithm/flowradar.h"
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
	uint32_t filter_bytes = 0;
	uint32_t memory_bytes = 0;
	if (argc != 3) {
		LOG_ERR("Invalid format which should be %s filter_memory(KB) memory_space(KB)\n", argv[0]);
	}
	else {
		filter_bytes = stoi(string(argv[1])) * 1024;
		memory_bytes = stoi(string(argv[2])) * 1024;
	}

	// Compute configuration
	uint32_t filter_space = filter_bytes / 4;
	uint32_t memory_space = memory_bytes / 4;
	uint32_t bf_hashnum = 1;
	uint32_t cbf_hashnum = 1;

	// 1. Simple test for debugging
	if (DEBUG) {
		sketch = new FlowRadar(filter_space, memory_space, bf_hashnum, cbf_hashnum);
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
	sketch = new FlowRadar(filter_space, memory_space, bf_hashnum, cbf_hashnum);
	next_sketch = new FlowRadar(filter_space, memory_space, bf_hashnum, cbf_hashnum);

	pktdrop_sketch = new FlowRadar(filter_space, memory_space, bf_hashnum, cbf_hashnum);
	pktdrop_tuplekey_vector = generate_pktdrop_dataset(tuplekey_vector, dropped_tuplekeys, 0.5);

	incorrect_routing_sketch = new FlowRadar(filter_space, memory_space, bf_hashnum, cbf_hashnum);
	incorrect_routing_tuplekey_vector = generate_pktdrop_dataset(tuplekey_vector, incorrect_routing_tuplekeys, 0.5, false);

	LOG_MSG("\nTest with CAIDA...\n");
	test();
	delete sketch;
	sketch = new FlowRadar(filter_space, memory_space, bf_hashnum, cbf_hashnum);
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
		sketch = new FlowRadar(filter_space, memory_space, bf_hashnum, cbf_hashnum);
		test();
		return 0;
	}
}
