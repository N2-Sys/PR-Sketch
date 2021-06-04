#include "tm.h"

c_traffic_matrix_t::c_traffic_matrix_t(uint32_t key_num, uint32_t time_interval_cnt) {
	if ((int32_t)key_num <= 0) {
		LOG_ERR(TM_LOG_PREFIX "c_traffic_matrix_t: invalid key_num %d\n", key_num);
	}

	this->key_num = key_num;
	this->time_interval_cnt = time_interval_cnt;
	tm = new c_slice_data_t[time_interval_cnt];
	if (tm == NULL) {
		LOG_ERR(TM_LOG_PREFIX "c_traffic_matrix_t: new slice data array failed\n");
	}
}

c_traffic_matrix_t::c_traffic_matrix_t(const char* load_file) {
	FILE* load_fd = fopen(load_file, "rb");
	if (load_fd == NULL) {
		LOG_ERR(TM_LOG_PREFIX "c_traffic_matrix_t: cannot open %s: %s\n", 
				load_file, strerror(errno));
	}

	// load key_num
	uint32_t read_cnt = fread(&key_num, sizeof(uint32_t), 1, load_fd);
	if (read_cnt != 1) {
		LOG_ERR(TM_LOG_PREFIX "c_traffic_matrix_t: invalid read element count %d "\
				"of key_num which should be %d: %s\n", 
				read_cnt, 1, strerror(errno));
	}

	// load time_interval_cnt
	read_cnt = fread(&time_interval_cnt, sizeof(uint32_t), 1, load_fd);
	if (read_cnt != 1) {
		LOG_ERR(TM_LOG_PREFIX "c_traffic_matrix_t: invalid read element count %d "\
				"of time_interval_cnt which should be %d: %s\n", 
				read_cnt, 1, strerror(errno));
	}

	// allocate space for send datas
	tm = new c_slice_data_t[time_interval_cnt];

	// load send_datas
	for (uint32_t i = 0; i < time_interval_cnt; i++) {
		// load source index cnt of current slice data
		uint32_t tmp_data_src_cnt = 0;
		read_cnt = fread(&tmp_data_src_cnt, sizeof(uint32_t), 1, load_fd);
		if (read_cnt != 1) {
			LOG_ERR(TM_LOG_PREFIX "c_traffic_matrix_t: invalid read element count %d "\
					"of slice_data_src_cnt which should be %d: %s\n", 
					read_cnt, 1, strerror(errno));
		}

		c_slice_data_t* tmp_slice_data_p = &tm[i];

		for (uint32_t j = 0; j < tmp_data_src_cnt; j++) {
			// load jth source key of time slice i
			index_t tmp_src_key = 0;
			read_cnt = fread(&tmp_src_key, sizeof(index_t), 1, load_fd);
			if (read_cnt != 1) {
				LOG_ERR(TM_LOG_PREFIX "c_traffic_matrix_t: invalid read element count %d "\
						"of tmp_src_key which should be %d: %s\n", 
						read_cnt, 1, strerror(errno));
			}

			// load size
			uint32_t tmp_size = 0;
			read_cnt = fread(&tmp_size, sizeof(uint32_t), 1, load_fd);
			if (read_cnt != 1) {
				LOG_ERR(TM_LOG_PREFIX "c_traffic_matrix_t: invalid read element count %d "\
						"of sparse counters serialization size which should be %d: %s\n", 
						read_cnt, 1, strerror(errno));
			}

			// load sparse counters
			uint8_t buf[tmp_size];
			read_cnt = fread(buf, tmp_size, 1, load_fd);
			if (read_cnt != 1) {
				LOG_ERR(TM_LOG_PREFIX "c_traffic_matrix_t: invalid read element count %d "\
						"of sparse counters buf which should be %d: %s\n", 
						read_cnt, 1, strerror(errno));
			}
			c_sparse_counters_t tmp_counters = c_sparse_counters_t(buf);

			// insert pair
			std::pair<index_t, c_sparse_counters_t> tmp_pair = 
				std::pair<index_t, c_sparse_counters_t>(tmp_src_key, tmp_counters);
			tmp_slice_data_p->insert(tmp_pair);
		}
	}

	fclose(load_fd);
}

// Index in flow_record_data is time slice
// Index in traffic_matrix is destination index
void c_traffic_matrix_t::update_tm(index_t src_index, index_t dst_index, c_sparse_counters_t flow_record_data) {
	if (src_index >= key_num) {
		LOG_ERR(TM_LOG_PREFIX "update_tm: invalid src index %d which should in the range of [0, %d)\n",
				src_index, key_num);
	}

	if (dst_index >= key_num) {
		LOG_ERR(TM_LOG_PREFIX "update_tm: invalid dst index %d which should in the range of [0, %d)\n",
				dst_index, key_num);
	}

	// make sure the time scale is smaller than time_interval_cnt
	flow_record_data.remove_larger_index(time_interval_cnt);
	
	std::map<index_t, counter_t> data = flow_record_data.get_data();
	for (std::map<index_t, counter_t>::const_iterator iter = data.begin(); iter != data.end(); iter++) {
		index_t tmp_time_index = iter->first;
		counter_t tmp_counter = iter->second;

		// The corresponding slice data for current time index
		c_slice_data_t &tmp_slice_data = tm[tmp_time_index];

		// No such source index
		if (tmp_slice_data.find(src_index) == tmp_slice_data.end()) {
			c_sparse_counters_t tmp_sparse_counters = c_sparse_counters_t();
			tmp_sparse_counters.update_counters(dst_index, tmp_counter);
			std::pair<index_t, c_sparse_counters_t> tmp_pair = 
				std::pair<index_t, c_sparse_counters_t>(src_index, tmp_sparse_counters);
			tmp_slice_data.insert(tmp_pair);
		}
		else {
			tmp_slice_data[src_index].update_counters(dst_index, tmp_counter);
		}
	}
}

// save format of traffic matrix: key_num time_interval_cnt send_data_0 ... send_data_(n-1)
// save format of send_data_i: send_data_key_num_i key_i0 size_i0 sparse_counters_i0 ... 
// 								key_i(m-1) size_i(m-1) sprase_coutenrs_i(m-1)
void c_traffic_matrix_t::save(const char* save_file) const {
	FILE* save_fd = fopen(save_file, "wb");
	if (save_fd == NULL) {
		LOG_ERR(TM_LOG_PREFIX "save: cannot open %s: %s\n", save_file, strerror(errno));
	}

	// save key_num
	fwrite(&key_num, sizeof(uint32_t), 1, save_fd);

	// save time_interval_cnt
	fwrite(&time_interval_cnt, sizeof(uint32_t), 1, save_fd);

	// save slice datas
	for (uint32_t i = 0; i < time_interval_cnt; i++) {
		const c_slice_data_t& tmp_data = tm[i];

		// save source index cnt of slice data
		uint32_t tmp_data_src_cnt = tmp_data.size();
		fwrite(&tmp_data_src_cnt, sizeof(uint32_t), 1, save_fd);

		// save slice_data_i
		for (c_slice_data_t::const_iterator iter = tmp_data.begin(); 
				iter != tmp_data.end(); iter++) {
			// save jth source index of time i
			fwrite(&iter->first, sizeof(index_t), 1, save_fd);

			// save size_ij
			uint32_t tmp_size = iter->second.get_serialization_size();
			fwrite(&tmp_size, sizeof(uint32_t), 1, save_fd);

			// save sparse_counters_ij
			uint8_t buf[tmp_size];
			iter->second.serialization(buf);
			fwrite(buf, tmp_size, 1, save_fd);
		}
	}

	fclose(save_fd);
}

c_traffic_matrix_t::~c_traffic_matrix_t() {
	delete []tm;
}

// for statistics

uint32_t c_traffic_matrix_t::get_valid_time_interval_cnt() const {
	std::set<index_t> valid_set;

	for (index_t i = 0; i < time_interval_cnt; i++) {
		const c_slice_data_t& tmp_slice_data = tm[i];
		uint32_t tmp_data_src_cnt = tmp_slice_data.size();
		if (tmp_data_src_cnt > 0) {
			valid_set.insert(i);
		}
	}

	return valid_set.size();
}
