#include "shkryleva_s_qsort_smerge/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <vector>

#include "shkryleva_s_qsort_smerge/common/include/common.hpp"

namespace shkryleva_s_qsort_smerge {

ShkrylevaSQSortSMergeMPI::ShkrylevaSQSortSMergeMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = std::vector<int>();
}

bool ShkrylevaSQSortSMergeMPI::ValidationImpl() {
  return true;
}

bool ShkrylevaSQSortSMergeMPI::PreProcessingImpl() {
  return true;
}

bool ShkrylevaSQSortSMergeMPI::RunImpl() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int total_size = 0;
  if (rank == 0) {
    total_size = static_cast<int>(GetInput().size());
  }

  MPI_Bcast(&total_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (total_size == 0) {
    GetOutput() = std::vector<int>();
    return true;
  }

  int base_chunk = total_size / size;
  int remainder = total_size % size;

  std::vector<int> send_counts(size, base_chunk);
  std::vector<int> send_displs(size, 0);

  for (int i = 0; i < remainder; ++i) {
    send_counts[i]++;
  }

  for (int i = 1; i < size; ++i) {
    send_displs[i] = send_displs[i - 1] + send_counts[i - 1];
  }

  int local_size = send_counts[rank];
  std::vector<int> local_data(local_size);

  if (rank == 0) {
    MPI_Scatterv(GetInput().data(), send_counts.data(), send_displs.data(), MPI_INT, local_data.data(), local_size,
                 MPI_INT, 0, MPI_COMM_WORLD);
  } else {
    MPI_Scatterv(nullptr, nullptr, nullptr, MPI_INT, local_data.data(), local_size, MPI_INT, 0, MPI_COMM_WORLD);
  }

  if (!local_data.empty()) {
    QuickSortIterative(local_data);
  }

  std::vector<int> result;

  if (rank == 0) {
    result.resize(total_size);
  }

  MPI_Gatherv(local_data.data(), local_size, MPI_INT, (rank == 0) ? result.data() : nullptr, send_counts.data(),
              send_displs.data(), MPI_INT, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    for (int i = 1; i < size; ++i) {
      int start = send_displs[i];
      int end = start + send_counts[i];
      std::inplace_merge(result.begin(), result.begin() + start, result.begin() + end);
    }
  }

  int result_size = 0;
  if (rank == 0) {
    result_size = static_cast<int>(result.size());
  }

  MPI_Bcast(&result_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (rank != 0) {
    result.resize(result_size);
  }

  if (result_size > 0) {
    MPI_Bcast(result.data(), result_size, MPI_INT, 0, MPI_COMM_WORLD);
  }

  GetOutput() = result;

  return true;
}

bool ShkrylevaSQSortSMergeMPI::PostProcessingImpl() {
  return true;
}

}  // namespace shkryleva_s_qsort_smerge
