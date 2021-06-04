#ifndef BASE_SKETCH_H
#define BASE_SKETCH_H

#include <stdint.h>
#include <vector>
#include <set>

#include "../tuple/tuple.h"

using namespace std;

class BaseSketch {
	public:
		virtual void update(tuple_key_t cur_key) = 0;
		virtual uint32_t estimate(tuple_key_t cur_key) = 0;
		virtual set<tuple_key_t> get_tuplekeys() = 0;
		virtual uint32_t get_transmission_pkts() = 0;
		virtual uint32_t get_transmission_bytes() = 0;
		virtual ~BaseSketch() {};
		virtual set<tuple_key_t> get_hcs(BaseSketch* other, uint32_t hc_threshold) { return set<tuple_key_t>(); }
};


#endif
