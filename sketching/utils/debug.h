#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>
#include <vector>
#include <set>
#include <map>

#include "./util.h"
#include "../tuple/tuple.h"

using namespace std;

// Other
void find_hh_threshold(vector<tuple_key_t> tuplekey_vector);
void find_hc_threshold(vector<tuple_key_t> tuplekey_vector, vector<tuple_key_t> next_tuplekey_vector);
void count_assumption_violation(vector<tuple_key_t> tuplekey_vector, uint32_t height, uint32_t width);
void count_assumption_violation2(vector<tuple_key_t> tuplekey_vector, uint32_t height, uint32_t width);

#endif
