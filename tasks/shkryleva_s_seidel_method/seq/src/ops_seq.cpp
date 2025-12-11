#include "shkryleva_s_seidel_method/seq/include/ops_seq.hpp"

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
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

  std::srand(static_cast<unsigned>(std::time(nullptr)));

  return true;
}

void ShkrylevaSSeidelMethodSEQ::generate_random_matrix(int size, std::vector<std::vector<double>> &matrix,
                                                       std::vector<double> &vector) const {
  matrix.resize(size, std::vector<double>(size, 0.0));
  vector.resize(size, 0.0);

  for (int i = 0; i < size; ++i) {
    double row_sum = 0.0;
    for (int j = 0; j < size; ++j) {
      if (i != j) {
        matrix[i][j] = static_cast<double>(std::rand() % 10 + 1) / 10.0;
        row_sum += std::abs(matrix[i][j]);
      }
    }
    matrix[i][i] = row_sum + static_cast<double>(std::rand() % 5 + 1);
    vector[i] = static_cast<double>(std::rand() % 20 + 1);
  }
}

bool ShkrylevaSSeidelMethodSEQ::converge(const std::vector<double> &x_new, const std::vector<std::vector<double>> &A,
                                         const std::vector<double> &b, double epsilon) const {
  double residual_norm = 0.0;
  int n = x_new.size();

  for (int i = 0; i < n; ++i) {
    double Ax_i = 0.0;
    for (int j = 0; j < n; ++j) {
      Ax_i += A[i][j] * x_new[j];
    }
    residual_norm += std::pow(Ax_i - b[i], 2);
  }

  return std::sqrt(residual_norm) < epsilon;
}

bool ShkrylevaSSeidelMethodSEQ::RunImpl() {
  int n = GetInput();
  if (n <= 0) {
    return false;
  }

  std::vector<std::vector<double>> A;
  std::vector<double> b;
  generate_random_matrix(n, A, b);

  std::vector<double> x(n, 0.0);

  for (int i = 0; i < n; ++i) {
    if (A[i][i] == 0.0) {
      return false;
    }
  }

  const double epsilon = 1e-6;
  const int max_iterations = 1000;
  int iteration = 0;
  bool converged = false;

  while (iteration < max_iterations) {
    double max_diff = 0.0;

    for (int i = 0; i < n; ++i) {
      double sum = b[i];

      for (int j = 0; j < n; ++j) {
        if (i != j) {
          sum -= A[i][j] * x[j];
        }
      }

      double new_xi = sum / A[i][i];
      double diff = std::abs(new_xi - x[i]);
      if (diff > max_diff) {
        max_diff = diff;
      }
      x[i] = new_xi;
    }

    if (max_diff < epsilon) {
      converged = true;
      break;
    }

    ++iteration;
  }

  double sum = 0.0;
  for (int i = 0; i < n; ++i) {
    sum += x[i];
  }

  if (converged) {
    GetOutput() = static_cast<int>(std::round(std::abs(sum)));
    if (GetOutput() == 0) {
      GetOutput() = 1;
    }
  } else {
    GetOutput() = -static_cast<int>(std::round(std::abs(sum)));
  }

  return true;
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
