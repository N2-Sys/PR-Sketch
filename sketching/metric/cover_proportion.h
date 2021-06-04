#ifndef COVER_PROPORTION_H
#define COVER_PROPORTION_H

#include <iostream>
#include <map>
#include <vector>
#include <set>

#include "../tuple/tuple.h"
#include "../utils/util.h"
#include "../utils/estimator.h"

using namespace std;

class CoverProportion {
	public:
		CoverProportion(const Estimator& estimator);

		void dump();
	private:
		uint32_t groundtruth_size = 0;
		map<tuple_key_t, float> estimation_error;
};

#endif
