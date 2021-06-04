#include "./univmon.h"

#include "../utils/util.h"
#include "../hash/hash.h"

Univmon::Univmon(uint32_t total_bytes, uint32_t layernum, uint32_t height, uint32_t heapsize_per_layer) {
	this->layernum = layernum;

	LOG_MSG("Heap size per layer: %d\n", heapsize_per_layer);
	uint32_t total_bytes_per_layer = total_bytes / layernum;

	for (uint32_t layer_idx = 0; layer_idx < layernum; layer_idx++) {
		countheap_vector.push_back(CountHeap(total_bytes_per_layer, height, heapsize_per_layer, layer_idx));
	}
}

void Univmon::update(tuple_key_t cur_key) {
	update(cur_key, false, 0, 1);
}

void Univmon::update(tuple_key_t cur_key, bool is_ns, uint32_t ns_row, uint32_t ns_value) {
	for (uint32_t layer_idx = 0; layer_idx < layernum; layer_idx++) {
		if (layer_idx != 0) {
			uint32_t sample_hash_idx = myhash(cur_key, layer_idx, 2).hash_idx;
			if (sample_hash_idx == 0) {
				break;
			}
		}

		countheap_vector[layer_idx].update(cur_key, is_ns, ns_row, ns_value);
	}
}

uint32_t Univmon::estimate(tuple_key_t cur_key) {
	/*uint32_t result = 0;
	for (int32_t layer_idx = layernum - 1; layer_idx >= 0; layer_idx--) {
		uint32_t estimation = countheap_vector[layer_idx].estimate(cur_key);
		if (layer_idx == layernum - 1) {
			result = estimation;
		}
		else {
			uint32_t sample_hash_idx = myhash(cur_key, layer_idx+1, 2).hash_idx;
			if (sample_hash_idx == 0) {
				result = 2 * result + estimation;
			}
			else {
				int32_t tmpv = 2 * result - estimation;
				if (tmpv < 0) {
					tmpv = 0;
				}
				result = uint32_t(tmpv);
			}
		}
	}
	return result;*/

	if (!is_decode) {
		decode();
	}

	if (decode_result.find(cur_key) != decode_result.end()) {
		return decode_result[cur_key];
	}
	else {
		return 0;
	}
}

set<tuple_key_t> Univmon::get_tuplekeys() {
	/*set<tuple_key_t> result;
	for (uint32_t layer_idx = 0; layer_idx < layernum; layer_idx++) {
		set<tuple_key_t> tmp = countheap_vector[layer_idx].get_tuplekeys();
		result.insert(tmp.begin(), tmp.end());
	}
	return result;*/

	if (!is_decode) {
		decode();
	}
	return decode_tuplekeys;
}

uint32_t Univmon::get_transmission_pkts() {
	uint32_t npkts = 0;
	for (uint32_t layer_idx = 0; layer_idx < layernum; layer_idx++) {
		npkts += countheap_vector[layer_idx].get_transmission_pkts();
	}
	return npkts;
}

uint32_t Univmon::get_transmission_bytes() {
	uint32_t nbytes = 0;
	for (uint32_t layer_idx = 0; layer_idx < layernum; layer_idx++) {
		nbytes += countheap_vector[layer_idx].get_transmission_bytes();
	}
	return nbytes;
}

void Univmon::decode() {
	LOG_MSG("Univmon decode...\n");
	for (int32_t layer_idx = layernum - 1; layer_idx >= 0; layer_idx--) {
		vector< std::pair<tuple_key_t, uint32_t> > tmp_heap = countheap_vector[layer_idx].get_heap();

		set<tuple_key_t> tmp_tuplekeys = countheap_vector[layer_idx].get_tuplekeys();
		decode_tuplekeys.insert(tmp_tuplekeys.begin(), tmp_tuplekeys.end());

		if (layer_idx == int32_t(layernum - 1)) {
			for (uint32_t i = 0; i < tmp_heap.size(); i++) {
				if (tmp_heap[i].second > 0) {
					decode_result[tmp_heap[i].first] = tmp_heap[i].second;
				}
			}
		}
		else {
			for (uint32_t i = 0; i < tmp_heap.size(); i++) {
				uint32_t prev_count = 0;
				if (decode_result.find(tmp_heap[i].first) != decode_result.end()) {
					prev_count = decode_result[tmp_heap[i].first];
				}

				uint32_t sample_hash_idx = myhash(tmp_heap[i].first, layer_idx+1, 2).hash_idx;
				if (sample_hash_idx == 0) {
					decode_result[tmp_heap[i].first] = 2 * prev_count + tmp_heap[i].second;
				}
				else {
					int32_t tmpv = 2 * prev_count - tmp_heap[i].second;
					if (tmpv < 0) {
						tmpv = 0;
					}
					decode_result[tmp_heap[i].first] = uint32_t(tmpv);
				}
			}
		}
	}

	is_decode = true;
}
