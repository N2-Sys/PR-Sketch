/*********************************************************************
 * Copyright (C)
 * File name: 	tm (traffic matrix)
 * Author: 		Siyuan Sheng
 * Version: 	1
 * Date: 		04/15/2019
 * Description: implement sparse traffic matrix measurement including
 * 				subset, host, app
**********************************************************************/

#ifndef __TM_H__
#define __TH_H__

#include <map>
#include <set>

#include "profile.h"
#include "../common/sparse_counters.h"

#define TM_LOG_PREFIX "[tm] "

// index_t: source key
// index_t of c_sparse_counters_t: destination key
// sparse_counters in traffic_matrix: dst_index -> counter
typedef std::map<index_t, c_sparse_counters_t> c_slice_data_t;

class c_traffic_matrix_t {
private:
	uint32_t key_num = 0; // row and column scale
	uint32_t time_interval_cnt = 0; // time scale
	c_slice_data_t* tm = NULL; // tm [time_interval_cnt]

public:
	c_traffic_matrix_t(uint32_t key_num, uint32_t time_interval_cnt);
	c_traffic_matrix_t(const char* load_file);
	~c_traffic_matrix_t();

	void update_tm(index_t time_index, index_t src_index, c_sparse_counters_t flow_record_data);
	void save(const char* save_file) const;

	// for statistics
	uint32_t get_valid_time_interval_cnt() const;
};

#endif
