#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <set>

/*#include <sys/wait.h>*/
/*#include <pthread.h>*/
#include <errno.h>

/*#include <semaphore.h>*/

#include "../common/util.h"
/*#include "ringbuffer.h"*/
#include "../common/packet.h"
#include "../common/config.h"
#include "../common/ancillary.h"
/*#include "cpu.h"*/

/*#include "hash.h"*/
/*#include "alg_keytbl.h"*/

#include "../preprocess/adapter_dir.h"

extern conf_t* conf;
extern struct PacketStat packet_stat;

const char* alg = NULL;
int is_output = 0;
int is_pin_cpu = 0;

int main (int argc, char *argv []) {

    if (argc != 2) {
    	fprintf(stderr, "Usage: %s [config file]\n", argv[0]);
    	exit(-1);
    }
   
    conf = Config_Init(argv[1]);

	const char* dir = conf_common_data_dir(conf);

    const char* pcap_list = conf_common_pcap_list(conf);
    adapter_t* adapter = adapter_init(dir, pcap_list);
	
    tuple_t t;
    memset(&t, 0, sizeof(struct Tuple));

    uint64_t start_ts = now_us();
    uint64_t cur_ts;

    const char* output_name = conf_common_record_file(conf);
    char tmp[100];
    sprintf(tmp, "%s\%s", dir, output_name);
    FILE* output = fopen(tmp, "wb");
    if (output == NULL) {
		LOG_ERR("cannot open %s: %s\n", output_name, strerror(errno));
    }

	//std::set<uint32_t> time_index_set;
	uint64_t pkt_start_ts = 0;
    while (1) {
    	enum PACKET_STATUS status;
    	if (adapter_next(adapter, &t, &status) == -1) {
    		break;
    	}

    	if (status != STATUS_VALID) {
    		continue;
    	}
		
		if (pkt_start_ts == 0) {
			pkt_start_ts = packet_stat.trace_start_ts;
		}

		/*uint32_t time_index = floor((t.pkt_ts - pkt_start_ts) / 300.0);*/
		/*time_index_set.insert(time_index);*/

    	fwrite(&t, sizeof(tuple_t), 1, output);
    }
	/*LOG_MSG("valid time interval cnt: %ld\n", time_index_set.size());*/
	/*for (std::set<uint32_t>::const_iterator iter = time_index_set.begin(); iter != time_index_set.end(); iter++) {*/
		/*LOG_MSG("time index: %d\n", *iter);*/
	/*}*/

    cur_ts = now_us();
    packet_stat.used_time += cur_ts - start_ts;
    report_final_stat();

    adapter_destroy(adapter);
    fclose(output);

	// save packet_stat
	const char* packet_stat_file = conf_common_packet_stat_file(conf);
	sprintf(tmp, "%s\%s", dir, packet_stat_file);
	FILE* pkt_stat_output = fopen(tmp, "wb");
	if (pkt_stat_output == NULL) {
		LOG_ERR("cannot open %s: %s\n", packet_stat_file, strerror(errno));
	}
	fwrite(&packet_stat, sizeof(PacketStat), 1, pkt_stat_output);
	fclose(pkt_stat_output);

    return 0;
}
