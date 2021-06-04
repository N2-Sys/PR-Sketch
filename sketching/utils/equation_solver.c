#include <stdlib.h>

#include "./equation_solver.h"
#include "./util.h"

vector<uint32_t> solve(vector<uint32_t> rows, vector<uint32_t> cols, vector<uint32_t> values, uint32_t nrow, uint32_t ncol, vector<uint32_t> sketch_vector) {
	afs_assert(rows.size()==cols.size(), "Invalid rows size; %lu, cols size: %lu\n", rows.size(), cols.size());
	afs_assert(rows.size()==values.size(), "Invalid rows size; %lu, values size: %lu\n", rows.size(), values.size());
	afs_assert(nrow==sketch_vector.size(), "Invalid nrow: %u, sketch size: %lu\n", nrow, sketch_vector.size());

	//LOG_MSG("Create coefficients...\n");
	vector<T> coefficients;
	uint32_t nval = values.size();
	for (uint32_t i = 0; i < nval; i++) {
		coefficients.push_back(T(rows[i], cols[i], double(values[i])));
	}

	//LOG_MSG("Build coefficient matrix...\n");
	SpMat A(nrow, ncol);
	A.setFromTriplets(coefficients.begin(), coefficients.end());

	//LOG_MSG("Build y...\n");
	V y(sketch_vector.size(), 1);
	for (uint32_t i = 0; i < sketch_vector.size(); i++) {
		y[i] = double(sketch_vector[i]);
	}

	//LOG_MSG("Create solver...\n");
	V x;
	//Solver solver;
	Solver2 solver;

	//LOG_MSG("Compute A...\n");
	solver.compute(A);
	if (solver.info() != Eigen::Success) {
		LOG_ERR("Compute A fail!\n");
	}

	//LOG_MSG("Solve x...\n");
	clock_t start = clock();
	x = solver.solve(y);
	clock_t end = clock();
	if (solver.info() != Eigen::Success) {
		LOG_ERR("Solve y fail!\n");
	}
	double time_cost = (end - start) / double(CLOCKS_PER_SEC) * 1000;
	LOG_MSG("Time cost of solving equation: %lf ms\n", time_cost);

	//LOG_MSG("Iteration num: %d, estimated error: %f\n", solver.iterations(), solver.error());

	//LOG_MSG("Convert result...\n");
	vector<uint32_t> result(ncol, 0);
	for (uint32_t i = 0; i < ncol; i++) {
		result[i] = uint32_t(x[i]+0.5); // Round
		/*if (i < 100) {
			LOG_MSG("i: %u, x: %lf, result: %u\n", i, x[i], result[i]);
		}*/
	}
	return result;
}

// For DEBUG
void equation_dump(vector<uint32_t> rows, vector<uint32_t> cols, vector<uint32_t> values, uint32_t nrow, uint32_t ncol, vector<uint32_t> sketch_vector) {
	afs_assert(rows.size()==cols.size(), "Invalid rows size; %lu, cols size: %lu\n", rows.size(), cols.size());
	afs_assert(rows.size()==values.size(), "Invalid rows size; %lu, values size: %lu\n", rows.size(), values.size());
	afs_assert(nrow==sketch_vector.size(), "Invalid nrow: %u, sketch size: %lu\n", nrow, sketch_vector.size());

	// Save matrix A
	FILE* fd = fopen("A.out", "w+");
	fprintf(fd, "%u %u %u\n", nrow, ncol, values.size());
	for (uint32_t i = 0; i < values.size(); i++) {
		fprintf(fd, "%u %u %u\n", rows[i]+1, cols[i]+1, values[i]);
	}
	fclose(fd);

	// Save vector y
	fd = fopen("y.out", "w+");
	fprintf(fd, "%u\n", sketch_vector.size());
	for (uint32_t i = 0; i < sketch_vector.size(); i++) {
		fprintf(fd, "%u\n", sketch_vector[i]);
	}
	fclose(fd);
}

// For Debug
vector<uint32_t> load_matlab_solution() {
	FILE* fd = fopen("x.out", "r");
	if (fd == NULL) {
		LOG_ERR("No such file: x.out!\n");
	}

	uint32_t length = 0;
	fscanf(fd, "%u\n", &length);

	vector<uint32_t> result(length, 0);
	for (uint32_t i = 0; i < length; i++) {
		double tmpvalue = 0;
		fscanf(fd, "%lf\n", &tmpvalue);
		result[i] = uint32_t(tmpvalue+0.5); // Round
	}
	return result;
}
