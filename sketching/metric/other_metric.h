#ifndef OTHER_METRIC_H
#define OTHER_METRIC_H

#include <set>
#include <map>
#include <stdint.h>

#include "../tuple/tuple.h"
#include "../utils/util.h"
#include "../utils/estimator.h"

void dump_FPR_FNR(const Estimator& estimator);
void dump_onesize_flow(const Estimator& estimator);
void dump_cardinality_re(const Estimator& estimator);
void dump_entropy_re(const Estimator& estimator);
uint32_t interset_cnt(const set<tuple_key_t>& a, const set<tuple_key_t>& b);

#endif
