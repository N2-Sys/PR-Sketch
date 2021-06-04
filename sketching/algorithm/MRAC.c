#include "./MRAC.h"

#include <cmath> // log() function uses e as the base

#include "../utils/util.h"
#include "../hash/hash.h"

CollisionGenerator::CollisionGenerator(uint32_t max_sum, uint32_t max_num) {
	this->max_sum = max_sum;
	this->max_num = max_num;

	LOG_MSG("Initialize collision data for dynamic programming...\n");
	DP_data.resize(max_num);
	for (uint32_t i = 0; i < max_num; i++) {
		DP_data[i].resize(max_sum);
	}

	LOG_MSG("Start dynamic programming...\n");
	for (uint32_t i = 0; i < max_num; i++) {
		uint32_t cur_num = i + 1;
		LOG_MSG("Start [%d / %d]\n", cur_num, max_num);
		for (uint32_t j = 0; j < max_sum; j++) {
			uint32_t cur_sum = j + 1;
			collisions_type_t* cur_vector = get_collisions(cur_num, cur_sum);
			if (cur_num == 1) { // Base case
				collision_type_t cur_collision;
				cur_collision.push_back(cur_sum);
				cur_vector->push_back(cur_collision);
			}
			else { // Iterative case
				uint32_t max_k = cur_sum / cur_num; // Assume that collision_type is ordered decreasingly
				if (max_k > MRAC_MAXK) {
					max_k = MRAC_MAXK;
				}
				for (uint32_t k = 1; k <= max_k; k++) { // The last element is k
					collisions_type_t* tmp_vector = get_collisions(cur_num-1, cur_sum-k); // Get cur_sum-k with cur_num-1 elements
					for (uint32_t collision_idx = 0; collision_idx < tmp_vector->size(); collision_idx++) {
						collision_type_t tmp_collision = (*tmp_vector)[collision_idx];
						if (check_collision(tmp_collision, k)) { // Avoid duplicate collisions
							collision_type_t cur_collision = tmp_collision;
							cur_collision.push_back(k);
							cur_vector->push_back(cur_collision);
						}
					}
				}
			}
		}
		LOG_MSG("Finish [%d / %d]\n", cur_num, max_num);
	}
	LOG_MSG("Finish dynamic programming!\n");
}

collisions_type_t CollisionGenerator::get_all_collisions(uint32_t cur_sum) {
	afs_assert(cur_sum<=max_sum, "Invalid sum: %d which is larger than max_sum %d\n", cur_sum, max_sum);
	collisions_type_t result;
	for (uint32_t i = 0; i < max_num; i++) {
		uint32_t cur_num = i + 1;
		collisions_type_t* tmp_vector = get_collisions(cur_num, cur_sum);
		if (tmp_vector->size() == 0) {
			break;
		}
		for (uint32_t j = 0; j < tmp_vector->size(); j++) {
			result.push_back((*tmp_vector)[j]);
		}
	}
	return result;
}

collisions_type_t* CollisionGenerator::get_collisions(uint32_t cur_num, uint32_t cur_sum) {
	afs_assert(cur_num<=max_num, "Invalid num: %d which is larger than max_num %d\n", cur_num, max_num);
	afs_assert(cur_sum<=max_sum, "Invalid sum: %d which is larger than max_sum %d\n", cur_sum, max_sum);
	collisions_type_t* result = &DP_data[cur_num - 1][cur_sum - 1];
	afs_assert(result!=NULL, "NULL pointer at (%u, %u)\n", cur_num-1, cur_sum-1);
	return result;
}

bool CollisionGenerator::check_collision(collision_type_t collision, uint32_t minv) {
	for (uint32_t i = 0; i < collision.size(); i++) {
		if (collision[i] < minv) {
			return false;
		}
	}
	return true;
}

void CollisionGenerator::append_collision(collision_type_t* cur_collision, collision_type_t other) {
	/*afs_assert(cur_collision!=NULL, "NULL pointer of cur_collision!\n");
	for (uint32_t i = 0; i < other.size(); i++) {
		(*cur_collision).push_back(other[i]);
	}*/
	cur_collision->insert(cur_collision->end(), other.begin(), other.end());
}

MRAC::MRAC(uint32_t total_bytes) {
	this->width = total_bytes / 4;

	buckets.resize(this->width, 0);
}

void MRAC::update(tuple_key_t cur_key) {
	uint32_t hashidx = myhash(cur_key, 0, width).hash_idx;
	buckets[hashidx] += 1;
}

uint32_t MRAC::estimate(tuple_key_t cur_key) {
	uint32_t hashidx = myhash(cur_key, 0, width).hash_idx;
	return buckets[hashidx];
}

set<tuple_key_t> MRAC::get_tuplekeys() {
	set<tuple_key_t> result;
	return result;
}

uint32_t MRAC::get_transmission_pkts() {
	uint32_t nbucket = buckets.size();
	uint32_t nbytes = nbucket * 4;
	uint32_t npkts = (nbytes-1)/1500 + 1;
	return npkts;
}

uint32_t MRAC::get_transmission_bytes(){
	uint32_t nbucket = buckets.size();
	uint32_t nbytes = nbucket * 4;
	return nbytes;
}

map<uint32_t, uint32_t> MRAC::get_flowsize_map() {
	if (!is_EM) {
		EM();
	}
	return EM_result;
}

void MRAC::EM() {
	LOG_MSG("Initialize model parameters...\n");
	// Initialize the skewed distribution: c*x^{-alpha} (continuous model)
	//uint32_t m = width; // the number of total buckets
	//uint32_t m0 = get_nbucket(0); // the number of buckets of zero
	//uint32_t n = uint32_t(double(m) * log(double(m)/double(m0))); // the number of total flows
	// x = 1: c = n1
	//uint32_t m1 = get_nbucket(1); // the number of buckets of one
	//uint32_t n1 = uint32_t(double(m1) * exp(double(n)/double(m))); // the number of flows of one
	//double c = n1;
	// x = 2: c*2^{-alpha} = n2 \approx m2
	//uint32_t m2 = get_nbucket(2); // the number of buckets of two
	//double alpha = log(c / double(m2)) / log(2);
	
	// Initialize the skewed distribution (discrete model)
	uint32_t z = get_maxv(); // maximum of potential flow size
	uint32_t n = buckets.size() - get_nbucket(0); // number of buckets of non-zero
	vector<double> phi_old(z, 0.0); // [1, z]
	vector<double> phi_new(z, 0.0); // [1, z]
	for (uint32_t i = 0; i < buckets.size(); i++) {
		if (buckets[i] > 0) {
			phi_new[buckets[i] - 1] += 1.0;
		}
	}
	for (uint32_t i = 0; i < phi_new.size(); i++) {
		phi_new[i] /= double(n);
	}

	/*LOG_MSG("z:%d\n", z);
	CollisionGenerator acollision_generator(10);
	vector<collision_type_t> tmp_collisions = acollision_generator.get_collisions(5);
	for (uint32_t i = 0; i < tmp_collisions.size(); i++) {
		for (uint32_t j = 0; j < tmp_collisions[i].size(); j++) {
			LOG_MSG("%d ", tmp_collisions[i][j]);
			if (j == tmp_collisions[i].size()-1) {
				LOG_MSG("\n");
			}
		}
	}
	exit(-1);*/

	// Expectation Maximization
	LOG_MSG("Create collision generator...\n");
	CollisionGenerator collision_generator(z);
	LOG_MSG("Start EM...\n");
	while (true) {
		// Expectation step
		LOG_MSG("Expectation step -> ");
		map<uint32_t, double> tmp_flowsize_map;
		for (uint32_t i = 1; i <= z; i++) {
			uint32_t mi = get_nbucket(i); // the number of buckets of value i
			if (mi == 0) {
				continue;
			}

			collisions_type_t tmp_collisions = collision_generator.get_all_collisions(i);
			for (uint32_t j = 0; j < tmp_collisions.size(); j++) {
				collision_type_t tmp_collision = tmp_collisions[j];
				double tmp_p = get_conditioned_probability(tmp_collision, phi_new, n, i, collision_generator); // Probability of Q(zi) in EM
				double tmp_deltan = double(mi) * tmp_p;
				for (uint32_t k = 0; k < tmp_collision.size(); k++) {
					uint32_t tmp_flowsize = tmp_collision[k];
					if (tmp_flowsize_map.find(tmp_flowsize) == tmp_flowsize_map.end()) {
						tmp_flowsize_map[tmp_flowsize] = tmp_deltan;
					}
					else {
						tmp_flowsize_map[tmp_flowsize] += tmp_deltan;
					}
				}
			}
		}

		// Maximization step
		LOG_MSG("Maximization step -> ");
		n = 0; // Update n
		for (map<uint32_t, double>::iterator iter = tmp_flowsize_map.begin(); iter != tmp_flowsize_map.end(); iter++) {
			n += uint32_t(iter->second);
		}
		phi_old = phi_new; // Update phi
		phi_new.assign(z, 0.0);
		for (map<uint32_t, double>::iterator iter = tmp_flowsize_map.begin(); iter != tmp_flowsize_map.end(); iter++) {
			phi_new[iter->first - 1] = double(uint32_t(iter->second)) / double(n);
		}

		// Check convergency
		double sum = 0.0;
		for (uint32_t i = 0; i < phi_new.size(); i++) {
			sum += abs(phi_new[i] - phi_old[i]);
		}
		LOG_MSG("convergency: %f\n", sum);
		if (sum < 0.1) {
			// Update result
			is_EM = true;
			for (map<uint32_t, double>::iterator iter = tmp_flowsize_map.begin(); iter != tmp_flowsize_map.end(); iter++) {
				uint32_t tmp_cnt = uint32_t(iter->second);
				if (tmp_cnt != 0) {
					EM_result[iter->first] = tmp_cnt;
				}
			}
			LOG_MSG("Finish EM!\n");
			break;
		}
	}
}

double MRAC::get_conditioned_probability(collision_type_t cur_collision, vector<double> phi, uint32_t n, uint32_t v, CollisionGenerator& collision_generator) {
	collisions_type_t all_collisions = collision_generator.get_all_collisions(v);
	double numerator = get_probability(cur_collision, phi, n);
	double denominator = 0.0;
	for (uint32_t i = 0; i < all_collisions.size(); i++) {
		denominator += get_probability(all_collisions[i], phi, n);	
	}
	return numerator / denominator;
}

double MRAC::get_probability(collision_type_t cur_collision, vector<double> phi, uint32_t n) {
	map<uint32_t, uint32_t> tmp_si_fi;
	for (uint32_t i = 0; i < cur_collision.size(); i++) {
		if (tmp_si_fi.find(cur_collision[i]) == tmp_si_fi.end()) {
			tmp_si_fi[cur_collision[i]] = 1;
		}
		else {
			tmp_si_fi[cur_collision[i]] += 1;
		}
	}

	double lambda = double(n) / double(width);
	double result = exp(-lambda);
	for (map<uint32_t, uint32_t>::iterator iter = tmp_si_fi.begin(); iter != tmp_si_fi.end(); iter++) {
		uint32_t si = iter->first;
		uint32_t fi = iter->second;
		double lambda_si = phi[si-1]*double(n)/double(width);
		double numerator = pow(lambda_si, fi);
		double denominator = 1.0; // fi!
		for (uint32_t i = 2; i <= fi; i++) {
			denominator *= double(i);
		}
		result *= (numerator / denominator);
	}
	return result;
}

uint32_t MRAC::get_nbucket(uint32_t value) {
	uint32_t result = 0;
	for (uint32_t i = 0; i < buckets.size(); i++) {
		if (buckets[i] == value) {
			result += 1;
		}
	}
	return result;
}

uint32_t MRAC::get_maxv() {
	uint32_t maxv = 0;
	for (uint32_t i = 0; i < buckets.size(); i++) {
		if (buckets[i] > maxv) {
			maxv = buckets[i];
		}
	}
	return maxv;
}
