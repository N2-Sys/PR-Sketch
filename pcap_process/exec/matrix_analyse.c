#include "../measure/tm.h"
#include "../common/nodekey_mapping.h"
#include "../common/config.h"

extern conf_t* conf;

int main(int argc, char *argv[]) {

	if (argc != 3) {
		LOG_ERR("Usage: %s [config file] [tm_type]\n", argv[0]);
	}

	const char* tm_type = argv[2];
	if (strcmp(tm_type, "subset") !=0 && strcmp(tm_type, "app") != 0 && strcmp(tm_type, "host") != 0) {
		LOG_ERR("Invalid tm_type %s which must be subset or host or app\n", argv[2]);
	}

	// get config information
	conf = Config_Init(argv[1]);
	const char* dir = conf_common_data_dir(conf);

	// load subset index mapping
	const char *subset_mapping_file = conf_common_subset_mapping_file(conf);
	const char *subset_mapping_path = path_join(dir, subset_mapping_file);
	c_nodekey_mapping_t<c_subset_key_t> subset_nodekey_mapping[3];
	for (uint32_t i = 0; i < 3; i++) {
		char tmp_path[100];
		sprintf(tmp_path, "%s.%d", subset_mapping_path, i+1);
		subset_nodekey_mapping[i] = c_nodekey_mapping_t<c_subset_key_t>(tmp_path);
	}

	// load host index mapping
	const char *host_mapping_file = conf_common_host_mapping_file(conf);
	const char *host_mapping_path = path_join(dir, host_mapping_file);
	c_nodekey_mapping_t<c_host_key_t> host_nodekey_mapping = c_nodekey_mapping_t<c_host_key_t>(host_mapping_path);

	// load app index mapping
	const char *app_mapping_file = conf_common_app_mapping_file(conf);
	const char *app_mapping_path = path_join(dir, app_mapping_file);
	c_nodekey_mapping_t<c_app_key_t> app_nodekey_mapping = c_nodekey_mapping_t<c_app_key_t>(app_mapping_path);

	// load traffic matrix
	const char* traffic_matrix_file = conf_common_traffic_matrix_file(conf);
	char tmp[MAX_STRING_LENGTH];
	sprintf(tmp, "%s_%s", tm_type, traffic_matrix_file);
	const char* traffic_matrix_path = path_join(dir, tmp);
	c_traffic_matrix_t *tm = NULL;
	c_traffic_matrix_t *tms[3];
	if (strcmp(tm_type, "subset")) {
		for (uint32_t i = 0; i < 3; i++) {
			char tmp_path [MAX_STRING_LENGTH];
			sprintf(tmp_path, "%s.%d", traffic_matrix_path, i+1);
			tms[i] = new c_traffic_matrix_t(tmp_path);
		}
	}
	else {
		tm = new c_traffic_matrix_t(traffic_matrix_path);
	}

	if (strcmp(tm_type, "subset")) {
		for (uint32_t i = 0; i < 3; i++) {
			// analyse valid time interval count
			uint32_t valid_cnt = tms[i]->get_valid_time_interval_cnt();
			LOG_MSG("subset %d valid time interval count: %d\n", i+1, valid_cnt);
		}
	}
	else {
		// analyse valid time interval count
		uint32_t valid_cnt = tm->get_valid_time_interval_cnt();
		LOG_MSG("valid time interval count: %d\n", valid_cnt);
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
