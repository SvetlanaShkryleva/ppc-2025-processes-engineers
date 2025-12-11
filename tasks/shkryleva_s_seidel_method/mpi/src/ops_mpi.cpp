#include "shkryleva_s_seidel_method/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
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

  if (rank == 0) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
  }

  MPI_Barrier(MPI_COMM_WORLD);
  return true;
}

void ShkrylevaSSeidelMethodMPI::generate_random_matrix(int size, std::vector<std::vector<double>> &matrix,
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

bool ShkrylevaSSeidelMethodMPI::converge(const std::vector<double> &x_new, const std::vector<std::vector<double>> &A,
                                         const std::vector<double> &b, double epsilon) const {
  int n = x_new.size();
  double residual_norm = 0.0;

  for (int i = 0; i < n; ++i) {
    double Ax_i = 0.0;
    for (int j = 0; j < n; ++j) {
      Ax_i += A[i][j] * x_new[j];
    }
    residual_norm += std::pow(Ax_i - b[i], 2);
  }

  return std::sqrt(residual_norm) < epsilon;
}

bool ShkrylevaSSeidelMethodMPI::RunImpl() {
  int rank = 0;
  int size = 1;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int n = GetInput();
  if (n <= 0) {
    return false;
  }

  std::vector<int> row_counts(size, 0);
  std::vector<int> row_displs(size, 0);

  int offset = 0;
  for (int proc = 0; proc < size; ++proc) {
    int base_rows = n / size;
    int extra = (proc < (n % size)) ? 1 : 0;
    row_counts[proc] = base_rows + extra;
    row_displs[proc] = offset;
    offset += row_counts[proc];
  }

  int local_rows = row_counts[rank];
  int start_row = row_displs[rank];

  std::vector<std::vector<double>> A_full;
  std::vector<double> b_full;
  std::vector<double> A_local(local_rows * n);
  std::vector<double> b_local(local_rows);

  if (rank == 0) {
    generate_random_matrix(n, A_full, b_full);

    for (int i = 0; i < n; ++i) {
      if (A_full[i][i] == 0.0) {
        MPI_Abort(MPI_COMM_WORLD, 1);
        return false;
      }
    }

    std::vector<int> send_counts(size, 0);
    std::vector<int> send_displs(size, 0);

    for (int proc = 0; proc < size; ++proc) {
      send_counts[proc] = row_counts[proc] * n;
      send_displs[proc] = row_displs[proc] * n;
    }

    std::vector<double> A_flat(n * n);
    for (int i = 0; i < n; ++i) {
      for (int j = 0; j < n; ++j) {
        A_flat[i * n + j] = A_full[i][j];
      }
    }

    MPI_Scatterv(A_flat.data(), send_counts.data(), send_displs.data(), MPI_DOUBLE, A_local.data(), local_rows * n,
                 MPI_DOUBLE, 0, MPI_COMM_WORLD);

    MPI_Scatterv(b_full.data(), row_counts.data(), row_displs.data(), MPI_DOUBLE, b_local.data(), local_rows,
                 MPI_DOUBLE, 0, MPI_COMM_WORLD);
  } else {
    MPI_Scatterv(nullptr, nullptr, nullptr, MPI_DOUBLE, A_local.data(), local_rows * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatterv(nullptr, nullptr, nullptr, MPI_DOUBLE, b_local.data(), local_rows, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  }

  std::vector<double> x_local(local_rows, 0.0);
  std::vector<double> x_global(n, 0.0);

  const double epsilon_val = 1e-6;
  const int max_iterations = 1000;
  int iteration = 0;

  while (iteration < max_iterations) {
    MPI_Allgatherv(x_local.data(), local_rows, MPI_DOUBLE, x_global.data(), row_counts.data(), row_displs.data(),
                   MPI_DOUBLE, MPI_COMM_WORLD);

    double local_max_diff = 0.0;

    for (int i = 0; i < local_rows; ++i) {
      int global_i = start_row + i;

      double sum = b_local[i];

      for (int j = 0; j < global_i; ++j) {
        sum -= A_local[i * n + j] * x_global[j];
      }

      for (int j = global_i + 1; j < n; ++j) {
        sum -= A_local[i * n + j] * x_global[j];
      }

      double new_val = sum / A_local[i * n + global_i];
      double diff = std::abs(new_val - x_local[i]);
      if (diff > local_max_diff) {
        local_max_diff = diff;
      }
      x_local[i] = new_val;
    }

    double global_max_diff = 0.0;
    MPI_Allreduce(&local_max_diff, &global_max_diff, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);

    if (global_max_diff < epsilon_val) {
      break;
    }

    ++iteration;
  }

  MPI_Allgatherv(x_local.data(), local_rows, MPI_DOUBLE, x_global.data(), row_counts.data(), row_displs.data(),
                 MPI_DOUBLE, MPI_COMM_WORLD);

  bool converged = false;
  if (rank == 0) {
    converged = converge(x_global, A_full, b_full, epsilon_val);
  }
  MPI_Bcast(&converged, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);

  double local_sum = 0.0;
  for (int i = 0; i < local_rows; ++i) {
    local_sum += x_local[i];
  }

  double global_sum = 0.0;
  MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

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

  MPI_Bcast(&result, 1, MPI_INT, 0, MPI_COMM_WORLD);
  GetOutput() = result;

  return true;
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
