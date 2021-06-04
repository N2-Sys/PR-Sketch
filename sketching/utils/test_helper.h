#include <iostream>
#include <string>
#include <vector>
#include <set>

#include "../tuple/tuple.h"
#include "../tuple/tuple_records.h"
#include "./util.h"
#include "./estimator.h"
#include "../algorithm/base_sketch.h"
#include "../hash/hash.h"

extern char label[];

extern vector<tuple_key_t> tuplekey_vector;
extern uint32_t bytesum;
extern BaseSketch* sketch;

extern vector<tuple_key_t> next_tuplekey_vector;
extern uint32_t next_bytesum;
extern BaseSketch* next_sketch;

extern vector<tuple_key_t> pktdrop_tuplekey_vector;
extern BaseSketch* pktdrop_sketch;
extern set<tuple_key_t> dropped_tuplekeys;

extern vector<tuple_key_t> incorrect_routing_tuplekey_vector;
extern BaseSketch* incorrect_routing_sketch;
extern set<tuple_key_t> incorrect_routing_tuplekeys;

void test();

vector<tuple_key_t> generate_debug_dataset();
void debug(BaseSketch* sketch);
vector<tuple_key_t> generate_synthetic_dataset(set<tuple_key_t>& tuplekey_set);

void update_sketch(BaseSketch* sketch, vector<tuple_key_t> tuplekey_vector);
void decoding_test(BaseSketch* sketch, set<tuple_key_t> tuplekey_set);
void transmission_test(BaseSketch* sketch, vector<tuple_key_t> tuplekey_vector, uint32_t bytesum);
void thpt_test(BaseSketch* sketch, vector<tuple_key_t> tuplekey_vector);

Estimator get_estimator(BaseSketch* sketch, vector<tuple_key_t> tuplekey_vector);
void accuracy_test(const Estimator& estimator);
void heavykey_test(const Estimator& estimator, const Estimator& other);
void heavykey_test(const Estimator& estimator, const Estimator& other, BaseSketch* sketch, BaseSketch* next_sketch);
void flowsize_test(const Estimator& estimator, const char* label);

// Anomaly detection
vector<tuple_key_t> generate_pktdrop_dataset(vector<tuple_key_t> tuplekey_vector, set<tuple_key_t>& dropped_tuplekeys, float drop_ratio, bool need_remain = true);
void pktdrop_test(const Estimator& estimator, const Estimator& other, set<tuple_key_t> dropped_tuplekeys);
void incorrect_routing_test(const Estimator& estimator, set<tuple_key_t> incorrect_routing_tuplekeys);
