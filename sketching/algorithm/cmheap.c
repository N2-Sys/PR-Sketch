#include <algorithm>

#include "./cmheap.h"

#include "../utils/util.h"
#include "../hash/hash.h"

CMHeap::CMHeap(uint32_t total_bytes, uint32_t height, uint32_t heapsize, uint32_t threshold) {
	this->threshold = threshold;
	this->heapsize = heapsize;
	this->height = height;
	width = (total_bytes - 12*heapsize) / 4 / height;

	buckets.resize(height*width);

	// Heap (hash table)
	tuple_key_t init_tuplekey;
	init_tuplekey.src_ip = 0;
	init_tuplekey.dst_ip = 0;
	heap_vector.resize(heapsize, std::pair<tuple_key_t, uint32_t>(init_tuplekey, 0));
}

void CMHeap::update(tuple_key_t cur_key) {
	// Update count sketch
	uint32_t estimation = 0;
	for (uint32_t row_idx = 0; row_idx < height; row_idx++) {
		hash_result_t hash_result = myhash(cur_key, row_idx, width);
		uint32_t *cur_bucket = get_bucket(row_idx, hash_result.hash_idx);
		
		// CM sketch
		(*cur_bucket) += 1;

		if ((row_idx == 0) || ((*cur_bucket) < estimation)) {
			estimation = (*cur_bucket);
		}
	}

	if (estimation <= threshold) {
		return;
	}

	// Update heap (hash table)
	uint32_t heap_idx = myhash(cur_key, 0, heapsize).hash_idx;
	if (heap_vector[heap_idx].first.src_ip == 0 && heap_vector[heap_idx].first.dst_ip == 0) {
		heap_vector[heap_idx].first.src_ip = cur_key.src_ip;
		heap_vector[heap_idx].first.dst_ip = cur_key.dst_ip;
		heap_vector[heap_idx].second = estimation;
	}
	else if (heap_vector[heap_idx].first.src_ip == cur_key.src_ip && \
			heap_vector[heap_idx].first.dst_ip == cur_key.dst_ip) {
		heap_vector[heap_idx].second = estimation;
	}
	else {
		if (heap_vector[heap_idx].second < estimation) {
			heap_vector[heap_idx].first.src_ip = cur_key.src_ip;
			heap_vector[heap_idx].first.dst_ip = cur_key.dst_ip;
			heap_vector[heap_idx].second = estimation;
		}
	}
	
	// Update heap (map)
	//uint32_t estimation = estimate(cur_key);
	//if (heap_map.size() < heapsize) {
		//heap_map[cur_key] = estimation;
	//}
	//else {
		//if (heap_map.find(cur_key) != heap_map.end()) {
			//heap_map[cur_key] = estimation;
		//}
		//else{
			// //map<tuple_key_t, uint32_t>::iterator min_iter = heap_map.begin();
			// //for (map<tuple_key_t, uint32_t>::iterator iter = heap_map.begin(); iter != heap_map.end(); iter++) {
			//  	//if (iter->second < min_iter->second) {
			//  		//min_iter = iter;
			// 	 //}
			// //} 

		//map<tuple_key_t, uint32_t>::iterator min_iter = min_element(heap_map.begin(), heap_map.end(), mycmp());
			//if (estimation > min_iter->second) {
				//heap_map.erase(min_iter);
				//heap_map[cur_key] = estimation;
			//}
		//}
	//}
}

uint32_t CMHeap::estimate(tuple_key_t cur_key) {
	int32_t result = 0;

	vector<int32_t> result_vector;
	for (uint32_t row_idx = 0; row_idx < height; row_idx++) {
		hash_result_t hash_result = myhash(cur_key, row_idx, width);
		uint32_t *cur_bucket = get_bucket(row_idx, hash_result.hash_idx);
		result_vector.push_back(*cur_bucket);
	}
	sort(result_vector.begin(), result_vector.end());

	// Count sketch
	/*uint32_t mid_idx = result_vector.size()/2;
	if (result_vector.size() % 2 == 0) {
		result = result_vector[mid_idx-1] + result_vector[mid_idx];
		result = result/2;
	}
	else {
		result = result_vector[mid_idx];
	}*/

	// CM sketch
	result = result_vector[0];
	
	if (result < 0) {
		result = 0;
	}
	return (uint32_t)result;
}

set<tuple_key_t> CMHeap::get_tuplekeys() {
	set<tuple_key_t> result;
	for (uint32_t i = 0; i < heap_vector.size(); i++) {
		if (heap_vector[i].first.src_ip != 0 || heap_vector[i].first.dst_ip != 0) {
			result.insert(heap_vector[i].first);
		}
	}
	return result;
}

uint32_t CMHeap::get_transmission_pkts() {
	uint32_t nbucket = buckets.size();
	uint32_t nbytes = nbucket*4 + heap_vector.size()*12;
	uint32_t npkts = (nbytes-1)/1500 + 1;
	return npkts;
}

uint32_t CMHeap::get_transmission_bytes() {
	uint32_t nbucket = buckets.size();
	uint32_t nbytes = nbucket*4 + heap_vector.size()*12;
	return nbytes;
}

uint32_t* CMHeap::get_bucket(uint32_t row, uint32_t col) {
	afs_assert(row >= 0 && row < height, "Invalid row: %d (%d)\n", row, height);
	afs_assert(col >= 0 && col < width, "Invalid col: %d (%d)\n", col, width);
	return &buckets[row*width + col];
}

vector< std::pair<tuple_key_t, uint32_t> > CMHeap::get_heap() {
	vector< std::pair<tuple_key_t, uint32_t> > result;

	// Heap (hash table)
	for (uint32_t i = 0; i < heap_vector.size(); i++) {
		if (heap_vector[i].first.src_ip != 0 || heap_vector[i].first.dst_ip != 0) {
			result.push_back(heap_vector[i]);
		}
	}
	
	// Heap (map)
	//for (map<tuple_key_t, uint32_t>::iterator iter = heap_map.begin(); iter != heap_map.end(); iter++) {
		//result.push_back(*iter);
	//}

	return result;
}
