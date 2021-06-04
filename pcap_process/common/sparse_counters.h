/*********************************************************************
 * Copyright (C)
 * File name: 	sparse_counter
 * Author: 		Siyuan Sheng
 * Version: 	1
 * Date: 		04/15/2019
 * Description: use a map to implement sparse counter array
**********************************************************************/

#ifndef __SPARSE_COUNTER_H__
#define __SPARSE_COUNTER_H__

#include <map>
#include <set>

#include "util.h"

#define SPARSE_COUNTER_LOG_PREFIX "[sparse counter] " 

typedef uint32_t index_t;

typedef struct __attribute__((__packed__)) Counter {
	uint32_t pkt_cnt;
	uint32_t byte_cnt;
} counter_t;

class c_sparse_counters_t {
private:
	std::map<index_t, counter_t> data;

public:
	c_sparse_counters_t();
	c_sparse_counters_t(uint8_t* buf);

	bool exists(index_t index) const;
	uint32_t get_number() const;
	std::set<index_t> get_indexes() const;
	uint32_t get_serialization_size() const;
	void update_counters(index_t index, counter_t counter);
	void update_counters(const c_sparse_counters_t &s);
	std::map<index_t, counter_t> get_data() const;
	void serialization(uint8_t* buf) const;

	void remove_larger_index(index_t upper);
	uint32_t get_byte_sparsity(uint32_t byte_threshold = 0) const;
	uint32_t get_pkt_sparsity(uint32_t pkt_threshold = 0) const;
};

#endif
