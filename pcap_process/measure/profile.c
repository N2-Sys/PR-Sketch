#include "profile.h"

static inline uint32_t get_time_interval_index(double start_ts, double end_ts) {
	if (end_ts < start_ts) {
		LOG_WARN(PROFILE_LOG_PREFIX "get_time_interval_index: "\
				"invalid end ts %lf for start ts %lf\n", start_ts, end_ts);
	}

	double result = (end_ts - start_ts) / double(TIME_INTERVAL);
	return floor(result);
}

// class c_subset_key_t

c_subset_key_t::c_subset_key_t(uint32_t ip, uint32_t prefix_len) {
	memset(ip_bytes, 0, sizeof(uint8_t)*4);
	this->prefix_len = prefix_len;

	// little endian
	uint8_t *tmp = (uint8_t*) &ip;
	for (uint32_t i = 0; i < prefix_len; i++) {
		ip_bytes[i] = tmp[i];
	}
}

c_subset_key_t::c_subset_key_t(uint8_t *buf) {
	if (buf == NULL) {
		LOG_ERR(PROFILE_LOG_PREFIX "c_subset_key_t: buf is null\n");
	}
	this->prefix_len = prefix_len;
	memcpy(&ip_bytes, buf, sizeof(uint8_t)*4);
}

bool c_subset_key_t::operator < (const c_subset_key_t &s) const {
	for (uint32_t i = 0; i < 4; i++) {
		if (ip_bytes[i] < s.ip_bytes[i]) return true;
		else if (ip_bytes[i] == s.ip_bytes[i]) continue;
		else return false;
	}
	return false;
}

void c_subset_key_t::serialization(uint8_t *buf) const {
	if (buf == NULL) {
		LOG_ERR(PROFILE_LOG_PREFIX "serialization: buf is null\n");
	}
	memcpy(buf, ip_bytes, sizeof(uint8_t)*4);
}

void c_subset_key_t::to_string(char *buf) const {
	if (buf == NULL) {
		LOG_ERR(PROFILE_LOG_PREFIX "to_string: buf is null\n");
	}
	sprintf(buf, "ip bytes [0]:%d [1]:%d [2]:%d [3]:%d", \
			ip_bytes[0], ip_bytes[1], ip_bytes[2], ip_bytes[3]);
}

// class c_host_key_t

c_host_key_t::c_host_key_t(uint32_t ip_param):ip(ip_param) {}

c_host_key_t::c_host_key_t(uint8_t* buf) {
	if (buf == NULL) {
		LOG_ERR(PROFILE_LOG_PREFIX "c_host_key_t: buf is null\n");
	}
	memcpy(&ip, buf, sizeof(uint32_t));
}

bool c_host_key_t::operator < (const c_host_key_t &s) const {
	return this->ip < s.ip;
}

void c_host_key_t::serialization(uint8_t* buf) const {
	if (buf == NULL) {
		LOG_ERR(PROFILE_LOG_PREFIX "serialization: buf is null\n");
	}
	memcpy(buf, &ip, sizeof(uint32_t));
}

void c_host_key_t::to_string(char* buf) const {
	if (buf == NULL) {
		LOG_ERR(PROFILE_LOG_PREFIX "to_string: buf is null\n");
	}
	const char* ip_str = ntoa(ip);
	sprintf(buf, "ip: %s", ip_str);
}

// class c_app_key_t

c_app_key_t::c_app_key_t(uint32_t ip_param, uint16_t port_param): 
	ip(ip_param), port(port_param) {}

c_app_key_t::c_app_key_t(uint8_t* buf) {
	if (buf == NULL) {
		LOG_ERR(PROFILE_LOG_PREFIX "c_app_key_t: buf is null\n");
	}
	memcpy(&ip, buf, sizeof(uint32_t));
	memcpy(&port, buf + sizeof(uint32_t), sizeof(uint32_t));
}

bool c_app_key_t::operator < (const c_app_key_t &s) const {
	return this->ip < s.ip || (this->ip == s.ip && this->port < s.port);
}

void c_app_key_t::serialization(uint8_t* buf) const {
	if (buf == NULL) {
		LOG_ERR(PROFILE_LOG_PREFIX "serialization: buf is null\n");
	}
	memcpy(buf, &ip, sizeof(uint32_t));
	memcpy(buf + sizeof(uint32_t), &port, sizeof(uint32_t));
}

void c_app_key_t::to_string(char* buf) const {
	if (buf == NULL) {
		LOG_ERR(PROFILE_LOG_PREFIX "to_string: buf is null\n");
	}
	const char* ip_str = ntoa(ip);
	sprintf(buf, "ip: %s port: %d", ip_str, port);
}

// class c_flow_key_t

c_flow_key_t::c_flow_key_t(const flow_key_t &key) {
	memcpy(&this->key, &key, sizeof(flow_key_t));
}

c_flow_key_t::c_flow_key_t(uint8_t* buf) {
	if (buf == NULL) {
		LOG_ERR(PROFILE_LOG_PREFIX "c_flow_key_t: buf is null\n");
	}
	memcpy(&key, buf, sizeof(flow_key_t));
}

bool c_flow_key_t::operator < (const c_flow_key_t &s) const {
	if (this->key.src_ip < s.key.src_ip) return true;
	else if (this->key.src_ip == s.key.src_ip) {
		if (this->key.dst_ip < s.key.dst_ip) return true;
		else if (this->key.dst_ip == s.key.dst_ip) {
			if (this->key.src_port < s.key.src_port) return true;
			else if (this->key.src_port == s.key.src_port) {
				if (this->key.dst_port < s.key.dst_port) return true;
				else if (this->key.dst_port == s.key.dst_port) {
					if (this->key.proto < s.key.proto) return true;
				}
			}
		}
	}
	return false;
}

void c_flow_key_t::serialization(uint8_t* buf) const { 
	if (buf == NULL) {
		LOG_ERR(PROFILE_LOG_PREFIX "serialization: buf is null\n");
	}
	memcpy(buf, &key, sizeof(flow_key_t));
}

void c_flow_key_t::to_string(char* buf) const {
	if (buf == NULL) {
		LOG_ERR(PROFILE_LOG_PREFIX "to_string: buf is null\n");
	}
	const char* src_ip_str = ntoa(key.src_ip);
	const char* dst_ip_str = ntoa(key.dst_ip);
	sprintf(buf, "src ip: %s src port: %d dst ip: %s dst port: %d", 
			src_ip_str, key.src_port, dst_ip_str, key.dst_port);
}

// class c_flow_record_t

c_flow_record_t::c_flow_record_t(double start_ts, double end_ts) {
	this->start_ts = start_ts;
	time_interval_cnt = get_time_interval_index(start_ts, end_ts) + 1;
	record.clear();
	LOG_MSG(PROFILE_LOG_PREFIX "c_flow_record_t: start_ts is %lf, end_ts is %lf, "\
			"interval count is %d\n", start_ts, end_ts, time_interval_cnt);
}

c_flow_record_t::c_flow_record_t(const char* load_file) {
	record.clear();

	FILE* load_fd = fopen(load_file, "rb");
	if (load_fd == NULL) {
		LOG_ERR(PROFILE_LOG_PREFIX "c_flow_record_t: cannot open %s: %s\n",
				load_file, strerror(errno));
	}

	//load start_ts
	uint32_t read_cnt = fread(&start_ts, sizeof(double), 1, load_fd);
	if (read_cnt != 1) {
		LOG_ERR(PROFILE_LOG_PREFIX "c_flow_record_t: invalid read element count "\
				"%d of start_ts which should be %d: %s\n", 
				read_cnt, 1, strerror(errno));
	}

	// load time_interval_cnt
	read_cnt = fread(&time_interval_cnt, sizeof(uint32_t), 1, load_fd);
	if (read_cnt != 1) {
		LOG_ERR(PROFILE_LOG_PREFIX "c_flow_record_t: invalid read element count "\
				"%d of time_interval_cnt which should be %d: %s\n", 
				read_cnt, 1, strerror(errno));
	}

	// load key_num
	uint32_t key_num = 0;
	read_cnt = fread(&key_num, sizeof(uint32_t), 1, load_fd);
	if (read_cnt != 1) {
		LOG_ERR(PROFILE_LOG_PREFIX "c_flow_record_t: invalid read element count "\
				"%d of key_num which should be %d: %s\n", 
				read_cnt, 1, strerror(errno));
	}

	// load <key - sparse counters> pairs
	uint32_t serialization_size = c_flow_key_t::serialization_size;
	uint8_t buf[serialization_size];
	memset(buf, 0, serialization_size);
	for (uint32_t i = 0; i < key_num; i++) {
		// load key
		read_cnt = fread(buf, serialization_size, 1, load_fd);
		if (read_cnt != 1) {
			LOG_ERR(PROFILE_LOG_PREFIX "c_flow_record_t: invalid read element count "\
					"%d of serialized flow key which should be %d: %s\n", 
					read_cnt, 1, strerror(errno));
		}
		c_flow_key_t tmp_key = c_flow_key_t(buf);

		// load size
		uint32_t tmp_size = 0;
		read_cnt = fread(&tmp_size, sizeof(uint32_t), 1, load_fd);
		if (read_cnt != 1) {
			LOG_ERR(PROFILE_LOG_PREFIX "c_flow_record_t: invalid read element count "\
					"%d of serialization size of sparse counters which should be %d: %s\n", 
					read_cnt, 1, strerror(errno));
		}

		// load counters
		uint8_t tmp_buf[tmp_size];
		read_cnt = fread(tmp_buf, tmp_size, 1, load_fd);
		if (read_cnt != 1) {
			LOG_ERR(PROFILE_LOG_PREFIX "c_flow_record_t: invalid read element count "\
					"%d of sparse counters which should be %d: %s\n", 
					read_cnt, 1, strerror(errno));
		}
		c_sparse_counters_t tmp_counters = c_sparse_counters_t(tmp_buf);

		// insert into record
		std::pair<c_flow_key_t, c_sparse_counters_t>tmp_pair = 
			std::pair<c_flow_key_t, c_sparse_counters_t>(tmp_key, tmp_counters);
		record.insert(tmp_pair);
	}

	fclose(load_fd);
}

bool c_flow_record_t::has_key(const c_flow_key_t &key) const {
	if (record.find(key) != record.end()) return true;
	else return false;
}

const std::map<c_flow_key_t, c_sparse_counters_t>& c_flow_record_t::get_record() const {
	return record;
}

void c_flow_record_t::update_record(const tuple_t &tuple) {
	c_flow_key_t key = c_flow_key_t(tuple.key);

	// no such flow key
	if (!has_key(key)) {
		// create a new sparse counters
		c_sparse_counters_t init_sparse_counters = c_sparse_counters_t();

		// insert new pair into record
		std::pair<c_flow_key_t, c_sparse_counters_t>init_pair = 
			std::pair<c_flow_key_t, c_sparse_counters_t>(key, init_sparse_counters);
		record.insert(init_pair);
	}

	// get corresponding time index
	index_t tmp_index = get_time_interval_index(start_ts, tuple.pkt_ts);
	if (tmp_index <0 || tmp_index >= time_interval_cnt) {
		LOG_WARN(PROFILE_LOG_PREFIX "update_record: invalid index %d out range of [0, %d), pkt_ts is %f\n", 
				tmp_index, time_interval_cnt, tuple.pkt_ts);
		return;
	}

	if (!has_key(key)) {
		char tmp[MAX_STRING_LENGTH];
		key.to_string(tmp);
		LOG_ERR(PROFILE_LOG_PREFIX "update_record: the key %s still doesn't exist\n", tmp);
	}

	// update corresponding counter info
	counter_t tmp_counter;
	memset(&tmp_counter, 0, sizeof(counter_t));
	tmp_counter.pkt_cnt = 1;
	tmp_counter.byte_cnt = tuple.size;
	record[key].update_counters(tmp_index, tmp_counter);
}

uint32_t c_flow_record_t::get_number() const {
	return record.size();
}

uint32_t c_flow_record_t::get_number(index_t index) const {
	uint32_t cnt = 0;
	for (std::map<c_flow_key_t, c_sparse_counters_t>::const_iterator iter = record.begin();
			iter != record.end(); iter++) {
		if (iter->second.exists(index)){
			cnt++;
		}
	}
	return cnt;
}

uint32_t c_flow_record_t::get_time_interval_cnt() const {
	return time_interval_cnt;
}

/* save format: start_ts time_interval_cnt \
 				key_num key_0 size_0 sparse-counters ... size_(n-1) key_(n-1) sparse-counters */
void c_flow_record_t::save(const char* save_file) const {
	FILE* save_fd = fopen(save_file, "wb");
	if (save_fd == NULL) {
		LOG_ERR(PROFILE_LOG_PREFIX "save: cannot open %s: %s\n", save_file, strerror(errno));
	}

	// save start_ts and time_interval_cnt
	LOG_MSG("Save start_ts and time_interval_cnt\n");
	fwrite(&start_ts, sizeof(double), 1, save_fd);
	fwrite(&time_interval_cnt, sizeof(uint32_t), 1, save_fd);

	// save key_num
	LOG_MSG("Save key_num\n");
	uint32_t key_num = record.size();
	fwrite(&key_num, sizeof(uint32_t), 1, save_fd);

	// save <key - sparse_counters> pairs
	LOG_MSG("Save key - sparse counters pairs\n");
	for (std::map<c_flow_key_t, c_sparse_counters_t>::const_iterator iter = record.begin();
			iter != record.end(); iter++) {
		// save key
		/*LOG_MSG("Save key\n");*/
		uint32_t key_size = c_flow_key_t::serialization_size;
		uint8_t key_buf[key_size];
		memset(key_buf, 0, key_size);
		iter->first.serialization(key_buf);
		fwrite(key_buf, key_size, 1, save_fd);

		// save size
		uint32_t tmp_size = iter->second.get_serialization_size();
		/*LOG_MSG("Save sparse counter whose size is %d\n", (int)tmp_size);*/
		fwrite(&tmp_size, sizeof(uint32_t), 1, save_fd);

		// save sparse counters
		/*LOG_MSG("Save sparse counters\n");*/
		uint8_t buf[tmp_size];
		iter->second.serialization(buf);
		fwrite(buf, tmp_size, 1, save_fd);
	}

	fclose(save_fd);
}
