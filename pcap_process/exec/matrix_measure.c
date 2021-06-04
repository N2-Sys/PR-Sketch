#include <errno.h>

#include "../measure/profile.h"
#include "../measure/tm.h"
#include "../common/nodekey_mapping.h"
#include "../common/ancillary.h"
#include "../common/config.h"

extern conf_t* conf;

int main (int argc, char *argv []) {

	if (argc != 3) {
		LOG_ERR("Usage: %s [config file] [tm_type]\n", argv[0]);
	}

	const char* tm_type = argv[2];
	if (strcmp(tm_type, "app") != 0 && strcmp(tm_type, "host") != 0 && strcmp(tm_type, "subset")) {
		LOG_ERR("Invalid tm_type %s which must be subset or host or app\n", argv[2]);
	}

	// get config information
	conf = Config_Init(argv[1]);
	const char* save_dir = conf_common_save_dir(conf);
	if (access(save_dir, 0) == -1) {
		LOG_ERR("No such dir %s\n", save_dir);
	}

	// load profile
	
	// load subset index mapping
	const char *subset_mapping_file = conf_common_subset_mapping_file(conf);
	const char *subset_mapping_path = path_join(save_dir, subset_mapping_file);
	c_nodekey_mapping_t<c_subset_key_t> subset_nodekey_mapping[3];
	for (uint32_t i = 0; i < 3; i++) {
		char tmp_path[100];
		sprintf(tmp_path, "%s.%d", subset_mapping_path, i+1);
		subset_nodekey_mapping[i] = c_nodekey_mapping_t<c_subset_key_t>(tmp_path);
	}
	
	// load host index mapping
	const char *host_mapping_file = conf_common_host_mapping_file(conf);
	const char *host_mapping_path = path_join(save_dir, host_mapping_file);
	c_nodekey_mapping_t<c_host_key_t> host_nodekey_mapping = c_nodekey_mapping_t<c_host_key_t>(host_mapping_path);

	// load app index mapping
	const char *app_mapping_file = conf_common_app_mapping_file(conf);
	const char *app_mapping_path = path_join(save_dir, app_mapping_file);
	c_nodekey_mapping_t<c_app_key_t> app_nodekey_mapping = c_nodekey_mapping_t<c_app_key_t>(app_mapping_path);

	// load flow record
	const char *flow_record_file = conf_common_flow_record_file(conf);
	const char *flow_record_path = path_join(save_dir, flow_record_file);
	c_flow_record_t flow_record = c_flow_record_t(flow_record_path);

	// create traffic matrix
	uint32_t key_num = 0;
	uint32_t key_nums[3];
	if (strcmp(tm_type, "subset") == 0) {
		for (uint32_t i = 0; i < 3; i++) {
			key_nums[i] = subset_nodekey_mapping[i].get_number();
		}
	}
	else if (strcmp(tm_type, "host") == 0) {
		key_num = host_nodekey_mapping.get_number();
	}
	else {
		key_num = app_nodekey_mapping.get_number();
	}

	c_traffic_matrix_t *tm = NULL;
	c_traffic_matrix_t *tms[3];
	uint32_t time_interval_cnt = flow_record.get_time_interval_cnt();
	if (strcmp(tm_type, "subset") == 0) {
		for (uint32_t i = 0; i < 3; i++) {
			tms[i] = new c_traffic_matrix_t(key_nums[i], time_interval_cnt);
		}
	}
	else {
		tm = new c_traffic_matrix_t(key_num, time_interval_cnt);
	}

	uint64_t start_ts = now_us();

	// build traffic matrix
	const std::map<c_flow_key_t, c_sparse_counters_t>& data = flow_record.get_record();
	for (std::map<c_flow_key_t, c_sparse_counters_t>::const_iterator iter = data.begin(); 
			iter != data.end(); iter++) {
		// get src index and dst index
		uint32_t src_index = 0;
		uint32_t dst_index = 0;
		uint32_t src_indexes[3];
		uint32_t dst_indexes[3];
		if (strcmp(tm_type, "subset") == 0) {
			for (uint32_t i = 0; i < 3; i++) {
				c_subset_key_t src_key = c_subset_key_t(iter->first.key.src_ip, i+1);
				src_indexes[i] = subset_nodekey_mapping[i].get_index_by_key(src_key);

				c_subset_key_t dst_key = c_subset_key_t(iter->first.key.dst_ip, i+1);
				dst_indexes[i] = subset_nodekey_mapping[i].get_index_by_key(dst_key);
			}
		}
		else if (strcmp(tm_type, "host") == 0) {
			c_host_key_t src_key = c_host_key_t(iter->first.key.src_ip);
			src_index = host_nodekey_mapping.get_index_by_key(src_key);

			c_host_key_t dst_key = c_host_key_t(iter->first.key.dst_ip);
			dst_index = host_nodekey_mapping.get_index_by_key(dst_key);
		}
		else {
			c_app_key_t src_key = c_app_key_t(iter->first.key.src_ip, iter->first.key.src_port);
			src_index = app_nodekey_mapping.get_index_by_key(src_key);

			c_app_key_t dst_key = c_app_key_t(iter->first.key.dst_ip, iter->first.key.dst_port);
			dst_index = app_nodekey_mapping.get_index_by_key(dst_key);
		}

		// update traffic matrix
		if (strcmp(tm_type, "subset") == 0) {
			for (uint32_t i = 0; i < 3; i++) {
				tms[i]->update_tm(src_indexes[i], dst_indexes[i], iter->second);
			}
		}
		else {
			tm->update_tm(src_index, dst_index, iter->second);
		}
	}

	uint64_t end_ts = now_us();
	LOG_MSG("time cost: %ld us\n", end_ts - start_ts);

	// save traffic matrix
	const char* traffic_matrix_file = conf_common_traffic_matrix_file(conf);
	char tmp[MAX_STRING_LENGTH];
	sprintf(tmp, "%s_%s", tm_type, traffic_matrix_file);
	const char* traffic_matrix_path = path_join(save_dir, tmp);
	if (strcmp(tm_type, "subset") == 0) {
		for (uint32_t i = 0; i < 3; i++) {
			char tmp_path [MAX_STRING_LENGTH];
			sprintf(tmp_path, "%s.%d", traffic_matrix_path, i+1);
			tms[i]->save(tmp_path);
		}
	}
	else {
		tm->save(traffic_matrix_path);
	}

	if (strcmp(tm_type, "subset") == 0) {
		for (uint32_t i = 0; i < 3; i++) {
			delete tms[i];
		}
	}
	else {
		delete tm;
	}
}

