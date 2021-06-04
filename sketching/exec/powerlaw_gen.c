// Generate power-law parameters for a given skewness

#include <stdint.h>
#include <math.h>
#include <string>
#include "../utils/util.h"

using namespace std;

float skewness_fit(float slope, float intercept) {
	afs_assert(slope>0, "Slope must be larger than 0\n");

	float p_threshold = 1.0/FLOWNUM;
	float x = 1;
	float EX = 0;
	float EX3 = 0;
	while (true) {
		float tmp_p = intercept*pow(x, -slope);
		if (tmp_p < p_threshold) {
			break;
		}

		EX += x*tmp_p;
		EX3 += pow(x, 3)*tmp_p;

		x += 1;
	}

	float sigma = 0;
	float skewness = 0;
	for (uint32_t i = 1; i < x; i++) {
		float tmp_p = intercept*pow(x, -slope);
		sigma += pow(i-EX, 2)*tmp_p;
	}
	sigma = sqrt(sigma);
	skewness = (EX3 - 3*EX*pow(sigma, 2) - pow(EX, 3)) / pow(sigma, 3);
	return skewness;
}

int main(int argc, char* argv[]) {
	float skewness;
	if (argc != 2) {
		LOG_ERR("Invalid format which should be %s skewness\n", argv[0]);
	}
	else {
		skewness = stoi(string(argv[1]));
	}

	float intercept = INTERCEPT;
	float slope = SLOPE;
	float tmp_skewness = skewness_fit(slope, intercept);
	bool is_smaller = tmp_skewness < skewness;
	LOG_MSG("tmp skewness: %f, target skewness: %f\n", tmp_skewness, skewness);

	float delta_slope = 0.01;
	while (true) {
		if (is_smaller) {
			slope += delta_slope;
		}
		else {
			slope -= delta_slope;
		}
		tmp_skewness = skewness_fit(slope, intercept);
		LOG_MSG("slope: %f, tmp skewness: %f, target skewness: %f\n", slope, tmp_skewness, skewness);
		if (is_smaller && tmp_skewness >= skewness) {
			break;
		}
		if (!is_smaller && tmp_skewness <= skewness) {
			break;
		}
	}
}
