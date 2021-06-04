#include <iostream>

#include "../tuple/tuple.h"
#include "./hash.h"
#include "./MurmurHash3.h"

using namespace std;

uint32_t seed_list[] = {2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107,109,113,127,131,137,139};
uint32_t seed_gap = 20;

uint64_t mmh3(tuple_key_t tuple_key, uint64_t seed){
	// extract srcip and dstip and combine them as uint64
	uint64_t k1, k2;
	k1 = (uint64_t)tuple_key.src_ip;
	k2 = (uint64_t)tuple_key.dst_ip;
	uint64_t combined_k;
	combined_k = (k1<<32)+k2;

	// murmur hash
	uint64_t key_len = 8;
	uint64_t mmh3_ret[2];
	MurmurHash3_x64_128((const char*)&combined_k, key_len, seed, mmh3_ret);
	return mmh3_ret[0];
}

uint32_t mymod(uint64_t value, uint32_t length) {
	int32_t result = value % length;
	if(result < 0) {
		result += length;
	}
	return uint32_t(result);
}

uint64_t myhash_raw_value(tuple_key_t tuple_key, uint32_t index, uint32_t seed) {
	uint64_t result = mmh3(tuple_key, seed_list[index + seed * seed_gap]);
	return result;
}

hash_result_t myhash(tuple_key_t tuple_key, uint32_t index, uint32_t length, uint32_t seed){
	uint64_t val = mmh3(tuple_key, seed_list[index + seed * seed_gap]);

	uint32_t hash_idx = mymod(val, length);

	uint32_t hash_val_idx = mymod(val, 101);
	float hash_val = -1.0 + 0.02*hash_val_idx;

	hash_result_t hash_result;
	hash_result.hash_idx = hash_idx;
	hash_result.hash_val = hash_val;
	return hash_result;
}
