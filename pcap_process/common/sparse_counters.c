#include "sparse_counters.h"

c_sparse_counters_t::c_sparse_counters_t() {
	data.clear();
}

c_sparse_counters_t::c_sparse_counters_t(uint8_t* buf) {
	if (buf == NULL) {
		LOG_ERR(SPARSE_COUNTER_LOG_PREFIX "c_sparse_counters_t: buf is NULL\n");
	}

	uint8_t* tmp_buf = buf;

	// load counter_num
	uint32_t counter_num = 0;
	memcpy(&counter_num, tmp_buf, sizeof(uint32_t));
	tmp_buf += sizeof(uint32_t);

	// load index - counter pairs
	for (uint32_t i = 0; i < counter_num; i++) {
		index_t tmp_index = 0;
		counter_t tmp_counter;
		memset(&tmp_counter, 0 ,sizeof(counter_t));

		// load index
		memcpy(&tmp_index, tmp_buf, sizeof(index_t));
		tmp_buf += sizeof(index_t);

		// load counter
		memcpy(&tmp_counter, tmp_buf, sizeof(counter_t));
		tmp_buf += sizeof(counter_t);

		this->update_counters(tmp_index, tmp_counter);
	}
}

bool c_sparse_counters_t::exists(index_t index) const {
	if (data.find(index) != data.end()){
		return true;
	}
	return false;
}

uint32_t c_sparse_counters_t::get_number() const {
	return data.size();
}


std::set<index_t> c_sparse_counters_t::get_indexes() const {
	std::set<index_t> indexes;
	for (std::map<index_t, counter_t>::const_iterator iter = data.begin();
			iter != data.end(); iter++) {
		indexes.insert(iter->first);
	}
	return indexes;
}

void c_sparse_counters_t::update_counters(index_t index, counter_t counter) {
	std::map<index_t, counter_t>::iterator iter = data.find(index);

	if (iter == data.end()) {
		std::pair<index_t, counter_t> tmp_pair = std::pair<index_t, counter_t>(index, counter);
		data.insert(tmp_pair);
	}

	else {
		iter->second.pkt_cnt += counter.pkt_cnt;
		iter->second.byte_cnt += counter.byte_cnt;
	}
}

void c_sparse_counters_t::update_counters(const c_sparse_counters_t &s) {
	for (std::map<index_t, counter_t>::const_iterator iter = s.data.begin();
			iter != s.data.end(); iter++) {
		this->update_counters(iter->first, iter->second);
	}
}

std::map<index_t, counter_t> c_sparse_counters_t::get_data() const {
	return data;
}

uint32_t c_sparse_counters_t::get_serialization_size() const {
	uint32_t space_size = sizeof(uint32_t) + data.size() * (sizeof(index_t) + sizeof(counter_t));
	return space_size;
}

// format: counter_num index_0 counter_0 ... index_(n-1) counter_(n-1)
void c_sparse_counters_t::serialization(uint8_t* buf) const {
	uint8_t* tmp_buf = buf;

	// save counter_num
	uint32_t counter_num = data.size();
	memcpy(tmp_buf, &counter_num, sizeof(uint32_t));
	tmp_buf += sizeof(uint32_t);

	// save index - counter pairs
	for (std::map<index_t, counter_t>::const_iterator iter = data.begin();
			iter != data.end(); iter++) {
		// save index
		memcpy(tmp_buf, &iter->first, sizeof(index_t));
		tmp_buf += sizeof(index_t);

		// save counter
		memcpy(tmp_buf, &iter->second, sizeof(counter_t));
		tmp_buf += sizeof(counter_t);
	}
}

void c_sparse_counters_t::remove_larger_index(index_t upper) {
	for (std::map<index_t, counter_t>::iterator iter = data.begin(); 
			iter != data.end(); iter++) {
		if (iter->first >= upper) {
			LOG_MSG(SPARSE_COUNTER_LOG_PREFIX "remove_larger_index: remove index %d "\
					"which is larger than upper bound %d\n", iter->first, upper);
			data.erase(iter);
		}
	}
}

uint32_t c_sparse_counters_t::get_byte_sparsity(uint32_t byte_threshold) const{
	uint32_t cnt = 0;
	for (std::map<index_t, counter_t>::const_iterator iter = data.begin(); 
			iter != data.end(); iter++) {
		if (iter->second.byte_cnt >= byte_threshold) {
				cnt++;
		}
	}
	return cnt;
}

uint32_t c_sparse_counters_t::get_pkt_sparsity(uint32_t pkt_threshold) const {
	uint32_t cnt = 0;
	for (std::map<index_t, counter_t>::const_iterator iter = data.begin(); 
			iter != data.end(); iter++) {
		if (iter->second.pkt_cnt >= pkt_threshold) {
				cnt++;
		}
	}
	return cnt;
}
