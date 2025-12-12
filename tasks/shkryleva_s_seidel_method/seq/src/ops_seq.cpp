#include "shkryleva_s_seidel_method/seq/include/ops_seq.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

#include "shkryleva_s_seidel_method/common/include/common.hpp"

namespace shkryleva_s_seidel_method {

ShkrylevaSSeidelMethodSEQ::ShkrylevaSSeidelMethodSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool ShkrylevaSSeidelMethodSEQ::ValidationImpl() {
  return (GetInput() > 0) && (GetOutput() == 0);
}

bool ShkrylevaSSeidelMethodSEQ::PreProcessingImpl() {
  GetOutput() = 0;
  return true;
}

void ShkrylevaSSeidelMethodSEQ::GenerateRandomMatrix(size_t size, std::vector<std::vector<double>> &matrix,
                                                     std::vector<double> &vector) {
  matrix.resize(size, std::vector<double>(size, 0.0));
  vector.resize(size, 0.0);

  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_real_distribution<double> dis_off_diag(0.1, 1.0);
  std::uniform_real_distribution<double> dis_diag_add(1.0, 5.0);
  std::uniform_real_distribution<double> dis_vector(1.0, 20.0);

  for (size_t i = 0; i < size; ++i) {
    double row_sum = 0.0;
    for (size_t j = 0; j < size; ++j) {
      if (i != j) {
        matrix[i][j] = dis_off_diag(gen);
        row_sum += std::abs(matrix[i][j]);
      }
    }
    matrix[i][i] = row_sum + dis_diag_add(gen);
    vector[i] = dis_vector(gen);
  }
}

[[nodiscard]] bool ShkrylevaSSeidelMethodSEQ::Converge(const std::vector<double> &x_new,
                                                       const std::vector<std::vector<double>> &a,
                                                       const std::vector<double> &b, double epsilon) {
  double residual_norm = 0.0;
  size_t n = x_new.size();

  for (size_t i = 0; i < n; ++i) {
    double ax_i = 0.0;
    for (size_t j = 0; j < n; ++j) {
      ax_i += a[i][j] * x_new[j];
    }
    residual_norm += std::pow(ax_i - b[i], 2);
  }

  return std::sqrt(residual_norm) < epsilon;
}

bool ShkrylevaSSeidelMethodSEQ::RunImpl() {
  int n_input = GetInput();
  if (n_input <= 0) {
    return false;
  }

  size_t n = static_cast<size_t>(n_input);

  std::vector<std::vector<double>> a;
  std::vector<double> b;
  GenerateRandomMatrix(n, a, b);

  for (size_t i = 0; i < n; ++i) {
    if (std::abs(a[i][i]) < 1e-12) {
      return false;
    }
  }

  const double kEpsilon = 1e-6;
  const int kMaxIterations = 1000;
  std::vector<double> x(n, 0.0);

  bool converged = SolveGaussSeidel(a, b, x, kEpsilon, kMaxIterations);

  double sum = CalculateSolutionSum(x);
  SetOutputFromSum(sum, converged);

  return true;
}

bool ShkrylevaSSeidelMethodSEQ::SolveGaussSeidel(const std::vector<std::vector<double>> &a,
                                                 const std::vector<double> &b, std::vector<double> &x, double epsilon,
                                                 int max_iterations) {
  size_t n = x.size();
  int iteration = 0;

  while (iteration < max_iterations) {
    double max_diff = 0.0;

    for (size_t i = 0; i < n; ++i) {
      double sum = b[i];

      for (size_t j = 0; j < n; ++j) {
        if (i != j) {
          sum -= a[i][j] * x[j];
        }
      }

      double new_xi = sum / a[i][i];
      double diff = std::abs(new_xi - x[i]);
      max_diff = std::max(diff, max_diff);
      x[i] = new_xi;
    }

    if (max_diff < epsilon) {
      return true;
    }

    ++iteration;
  }

  return false;
}

double ShkrylevaSSeidelMethodSEQ::CalculateSolutionSum(const std::vector<double> &x) {
  double sum = 0.0;
  for (double value : x) {
    sum += value;
  }
  return sum;
}

void ShkrylevaSSeidelMethodSEQ::SetOutputFromSum(double sum, bool converged) {
  int rounded_sum = static_cast<int>(std::round(std::abs(sum)));

  if (converged) {
    GetOutput() = (rounded_sum == 0) ? 1 : rounded_sum;
  } else {
    GetOutput() = -rounded_sum;
  }
}

bool ShkrylevaSSeidelMethodSEQ::PostProcessingImpl() {
  if (GetOutput() < 0) {
    GetOutput() = -GetOutput();
  }
  if (GetOutput() == 0) {
    GetOutput() = 1;
  }
  return true;
}

}  // namespace shkryleva_s_seidel_method
