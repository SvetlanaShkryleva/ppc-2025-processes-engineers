#include "shkryleva_s_seidel_method/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

#include "shkryleva_s_seidel_method/common/include/common.hpp"

namespace shkryleva_s_seidel_method {

ShkrylevaSSeidelMethodMPI::ShkrylevaSSeidelMethodMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool ShkrylevaSSeidelMethodMPI::ValidationImpl() {
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int is_valid = 0;
  if (rank == 0) {
    is_valid = ((GetInput() > 0) && (GetOutput() == 0)) ? 1 : 0;
  }
  MPI_Bcast(&is_valid, 1, MPI_INT, 0, MPI_COMM_WORLD);

  return is_valid != 0;
}

bool ShkrylevaSSeidelMethodMPI::PreProcessingImpl() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (size < 1) {
    return false;
  }

  GetOutput() = 0;
  MPI_Barrier(MPI_COMM_WORLD);
  return true;
}

void ShkrylevaSSeidelMethodMPI::GenerateRandomMatrix(size_t size, std::vector<std::vector<double>> &matrix,
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

[[nodiscard]] bool ShkrylevaSSeidelMethodMPI::Converge(const std::vector<double> &x_new,
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

bool ShkrylevaSSeidelMethodMPI::RunImpl() {
  int rank = 0;
  int size = 1;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int n_input = GetInput();
  if (n_input <= 0) {
    return false;
  }

  size_t n = static_cast<size_t>(n_input);

  std::vector<int> row_counts(size, 0);
  std::vector<int> row_displs(size, 0);

  CalculateRowDistribution(n, size, row_counts, row_displs);

  int local_rows = row_counts[rank];
  int start_row = row_displs[rank];

  std::vector<double> a_local(local_rows * n);
  std::vector<double> b_local(local_rows);
  std::vector<double> x_local(local_rows, 0.0);
  std::vector<double> x_global(n, 0.0);

  bool has_error = false;
  if (rank == 0) {
    std::vector<std::vector<double>> a_full;
    std::vector<double> b_full;
    GenerateRandomMatrix(n, a_full, b_full);

    has_error = CheckDiagonalElements(a_full);
    ScatterData(a_full, b_full, a_local, b_local, row_counts, row_displs, size, n);
  } else {
    has_error = ReceiveScatteredData(a_local, b_local, local_rows, n);
  }

  if (CheckForErrors(has_error)) {
    return false;
  }

  bool converged =
      SolveIteratively(a_local, b_local, x_local, x_global, row_counts, row_displs, local_rows, start_row, n);

  double global_sum = CalculateGlobalSum(x_local, rank);

  int result = CalculateResult(global_sum, converged, rank, n);

  BroadcastResult(result);
  GetOutput() = result;

  return true;
}

void ShkrylevaSSeidelMethodMPI::CalculateRowDistribution(size_t n, int size, std::vector<int> &row_counts,
                                                         std::vector<int> &row_displs) {
  int offset = 0;
  for (int proc = 0; proc < size; ++proc) {
    int base_rows = static_cast<int>(n) / size;
    int extra = (proc < (static_cast<int>(n) % size)) ? 1 : 0;
    row_counts[proc] = base_rows + extra;
    row_displs[proc] = offset;
    offset += row_counts[proc];
  }
}

bool ShkrylevaSSeidelMethodMPI::CheckDiagonalElements(const std::vector<std::vector<double>> &a) {
  for (const auto &row : a) {
    size_t idx = &row - &a[0];
    if (std::abs(row[idx]) < 1e-12) {
      return true;
    }
  }
  return false;
}

void ShkrylevaSSeidelMethodMPI::ScatterData(const std::vector<std::vector<double>> &a_full,
                                            const std::vector<double> &b_full, std::vector<double> &a_local,
                                            std::vector<double> &b_local, const std::vector<int> &row_counts,
                                            const std::vector<int> &row_displs, int size, size_t n) {
  std::vector<int> send_counts(size, 0);
  std::vector<int> send_displs(size, 0);

  for (int proc = 0; proc < size; ++proc) {
    send_counts[proc] = row_counts[proc] * static_cast<int>(n);
    send_displs[proc] = row_displs[proc] * static_cast<int>(n);
  }

  std::vector<double> a_flat(n * n);
  for (size_t i = 0; i < n; ++i) {
    for (size_t j = 0; j < n; ++j) {
      a_flat[i * n + j] = a_full[i][j];
    }
  }

  MPI_Scatterv(a_flat.data(), send_counts.data(), send_displs.data(), MPI_DOUBLE, a_local.data(),
               static_cast<int>(a_local.size()), MPI_DOUBLE, 0, MPI_COMM_WORLD);
  MPI_Scatterv(b_full.data(), row_counts.data(), row_displs.data(), MPI_DOUBLE, b_local.data(),
               static_cast<int>(b_local.size()), MPI_DOUBLE, 0, MPI_COMM_WORLD);
}

bool ShkrylevaSSeidelMethodMPI::ReceiveScatteredData(std::vector<double> &a_local, std::vector<double> &b_local,
                                                     int local_rows, size_t n) {
  MPI_Scatterv(nullptr, nullptr, nullptr, MPI_DOUBLE, a_local.data(), local_rows * static_cast<int>(n), MPI_DOUBLE, 0,
               MPI_COMM_WORLD);
  MPI_Scatterv(nullptr, nullptr, nullptr, MPI_DOUBLE, b_local.data(), local_rows, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  return false;
}

bool ShkrylevaSSeidelMethodMPI::CheckForErrors(bool has_error) {
  int global_error = 0;
  MPI_Allreduce(&has_error, &global_error, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
  return global_error != 0;
}

bool ShkrylevaSSeidelMethodMPI::SolveIteratively(const std::vector<double> &a_local, const std::vector<double> &b_local,
                                                 std::vector<double> &x_local, std::vector<double> &x_global,
                                                 const std::vector<int> &row_counts, const std::vector<int> &row_displs,
                                                 int local_rows, int start_row, size_t n) {
  const double kEpsilon = 1e-6;
  const int kMaxIterations = 1000;
  int iteration = 0;

  while (iteration < kMaxIterations) {
    MPI_Allgatherv(x_local.data(), local_rows, MPI_DOUBLE, x_global.data(), row_counts.data(), row_displs.data(),
                   MPI_DOUBLE, MPI_COMM_WORLD);

    double local_max_diff = 0.0;

    for (int i = 0; i < local_rows; ++i) {
      int global_i = start_row + i;

      double sum = b_local[i];

      for (size_t j = 0; j < static_cast<size_t>(global_i); ++j) {
        sum -= a_local[i * static_cast<int>(n) + static_cast<int>(j)] * x_global[j];
      }

      for (size_t j = static_cast<size_t>(global_i) + 1; j < n; ++j) {
        sum -= a_local[i * static_cast<int>(n) + static_cast<int>(j)] * x_global[j];
      }

      double new_val = sum / a_local[i * static_cast<int>(n) + global_i];
      double diff = std::abs(new_val - x_local[i]);
      local_max_diff = std::max(diff, local_max_diff);
      x_local[i] = new_val;
    }

    double global_max_diff = 0.0;
    MPI_Allreduce(&local_max_diff, &global_max_diff, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);

    if (global_max_diff < kEpsilon) {
      return true;
    }

    ++iteration;
  }

  return false;
}

double ShkrylevaSSeidelMethodMPI::CalculateGlobalSum(const std::vector<double> &x_local, int rank) {
  double local_sum = 0.0;
  for (double value : x_local) {
    local_sum += value;
  }

  double global_sum = 0.0;
  MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  return global_sum;
}

int ShkrylevaSSeidelMethodMPI::CalculateResult(double global_sum, bool converged, int rank, size_t n) {
  int result = 0;
  if (rank == 0) {
    if (converged) {
      if (std::abs(global_sum) < 0.0001) {
        result = 1;
      } else {
        result = static_cast<int>(std::round(std::abs(global_sum)));
      }
    } else {
      result = -static_cast<int>(std::round(std::abs(global_sum)));
    }
  }
  return result;
}

void ShkrylevaSSeidelMethodMPI::BroadcastResult(int &result) {
  MPI_Bcast(&result, 1, MPI_INT, 0, MPI_COMM_WORLD);
}

bool ShkrylevaSSeidelMethodMPI::PostProcessingImpl() {
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (GetOutput() < 0) {
    GetOutput() = -GetOutput();
  }
  if (GetOutput() == 0) {
    GetOutput() = 1;
  }

  return true;
}

}  // namespace shkryleva_s_seidel_method
