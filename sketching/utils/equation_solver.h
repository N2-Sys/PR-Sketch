#include <Eigen/SparseQR>
#include <Eigen/IterativeLinearSolvers>

#include <vector>

using namespace std;

typedef Eigen::SparseQR<Eigen::SparseMatrix<double>, Eigen::COLAMDOrdering<int>> Solver;
typedef Eigen::LeastSquaresConjugateGradient<Eigen::SparseMatrix<double>> Solver2;
typedef Eigen::SparseMatrix<double> SpMat; // declares a column-major sparse matrix type of double
typedef Eigen::Triplet<double> T;
typedef Eigen::VectorXd V; // VectorXd = Matrix<double, Dynamic, 1>

vector<uint32_t> solve(vector<uint32_t> rows, vector<uint32_t> cols, vector<uint32_t> values, uint32_t nrow, uint32_t ncol, vector<uint32_t> sketch_vector);
void equation_dump(vector<uint32_t> rows, vector<uint32_t> cols, vector<uint32_t> values, uint32_t nrow, uint32_t ncol, vector<uint32_t> sketch_vector);
vector<uint32_t> load_matlab_solution();
