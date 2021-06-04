#ifndef TUPLE_RECORDS
#define TUPLE_RECORDS

#include <vector>
#include <string>
#include <iostream>

#include "./tuple.h"
#include "../utils/util.h"

using namespace std;

class TupleRecords{
	public:
		TupleRecords(string filepath);
		vector<tuple_t> read_all(); // For network data (tuples)
		vector<tuple_key_t> read_all_tuplekey();
		uint32_t get_bytesum();
	private:
		string filepath;

		vector<tuple_t> read_all_clicks(); // For click data
};

#endif
