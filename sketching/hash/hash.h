#ifndef HASH
#define HASH

#include <iostream>

#include "../tuple/tuple.h"

using namespace std;

typedef struct __attribute__((__packed__)) HashResult{
	uint32_t hash_idx;
	float hash_val;
} hash_result_t;

uint32_t mymod(uint64_t value, uint32_t length);
uint64_t myhash_raw_value(tuple_key_t tuple_key, uint32_t index, uint32_t seed = 0);
hash_result_t myhash(tuple_key_t tuple_key, uint32_t index, uint32_t length, uint32_t seed = 0);

#endif
