#include "shkryleva_s_qsort_smerge/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cstddef>
#include <iostream>
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
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  GetOutput() = std::vector<int>();

  MPI_Barrier(MPI_COMM_WORLD);
  return true;
}

bool ShkrylevaSQSortSMergeMPI::RunImpl() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  std::vector<int> global_data;
  int global_size = 0;

  if (rank == 0) {
    global_data = GetInput();
    global_size = static_cast<int>(global_data.size());
  }

  MPI_Bcast(&global_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (global_size == 0) {
    if (rank == 0) {
      GetOutput() = std::vector<int>();
    }
    MPI_Barrier(MPI_COMM_WORLD);
    return true;
  }

  std::vector<int> counts(size, 0);
  std::vector<int> displs(size, 0);

  int base = global_size / size;
  int remainder = global_size % size;
  int offset = 0;

  for (int i = 0; i < size; ++i) {
    counts[i] = base + (i < remainder ? 1 : 0);
    displs[i] = offset;
    offset += counts[i];
  }

  int local_size = counts[rank];
  std::vector<int> local_data(local_size);

  MPI_Scatterv(rank == 0 ? global_data.data() : nullptr, counts.data(), displs.data(), MPI_INT, local_data.data(),
               local_size, MPI_INT, 0, MPI_COMM_WORLD);

  std::sort(local_data.begin(), local_data.end());

  std::vector<int> gathered_data;
  if (rank == 0) {
    gathered_data.resize(global_size);
  }

  MPI_Gatherv(local_data.data(), local_size, MPI_INT, rank == 0 ? gathered_data.data() : nullptr, counts.data(),
              displs.data(), MPI_INT, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    if (size == 1) {
      GetOutput() = gathered_data;
    } else {
      std::vector<int> result;

      int first_proc = 0;
      while (first_proc < size && counts[first_proc] == 0) {
        first_proc++;
      }

      if (first_proc < size) {
        int start_idx = displs[first_proc];
        int end_idx = start_idx + counts[first_proc];
        result = std::vector<int>(gathered_data.begin() + start_idx, gathered_data.begin() + end_idx);

        for (int i = first_proc + 1; i < size; ++i) {
          if (counts[i] > 0) {
            int part_start = displs[i];
            int part_end = part_start + counts[i];
            std::vector<int> part(gathered_data.begin() + part_start, gathered_data.begin() + part_end);

            std::vector<int> merged(result.size() + part.size());
            std::merge(result.begin(), result.end(), part.begin(), part.end(), merged.begin());
            result = std::move(merged);
          }
        }
      }

      GetOutput() = result;
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);
  return true;
}

bool ShkrylevaSQSortSMergeMPI::PostProcessingImpl() {
  return true;
}

void ShkrylevaSQSortSMergeMPI::ComputeDistribution(int n, int size, std::vector<int> &counts,
                                                   std::vector<int> &displs) {
  counts.assign(size, 0);
  displs.assign(size, 0);

  int base = n / size;
  int remainder = n % size;
  int offset = 0;

  for (int i = 0; i < size; ++i) {
    counts[i] = base + (i < remainder ? 1 : 0);
    displs[i] = offset;
    offset += counts[i];
  }
}

std::vector<int> ShkrylevaSQSortSMergeMPI::Merge(const std::vector<int> &left, const std::vector<int> &right) {
  std::vector<int> result;
  result.reserve(left.size() + right.size());

  std::merge(left.begin(), left.end(), right.begin(), right.end(), std::back_inserter(result));

  return result;
}

std::vector<int> ShkrylevaSQSortSMergeMPI::QuickSortWithMerge(const std::vector<int> &arr) {
  // Простая реализация с использованием std::sort
  std::vector<int> result = arr;
  std::sort(result.begin(), result.end());
  return result;
}

}  // namespace shkryleva_s_qsort_smerge
