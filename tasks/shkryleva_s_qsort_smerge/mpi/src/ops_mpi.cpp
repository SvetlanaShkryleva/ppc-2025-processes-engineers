#include "shkryleva_s_qsort_smerge/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cstddef>
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
  MPI_Barrier(MPI_COMM_WORLD);
  return true;
}

bool ShkrylevaSQSortSMergeMPI::RunImpl() {
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  std::vector<int> input;
  int n = 0;

  if (rank == 0) {
    input = GetInput();
    n = static_cast<int>(input.size());
  }

  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (n == 0) {
    if (rank == 0) {
      GetOutput() = std::vector<int>();
    }
    MPI_Barrier(MPI_COMM_WORLD);
    return true;
  }

  std::vector<int> all_data(n);

  if (rank == 0) {
    all_data = input;
  }

  MPI_Bcast(all_data.data(), n, MPI_INT, 0, MPI_COMM_WORLD);

  std::vector<int> counts(size);
  std::vector<int> displs(size);

  ComputeDistribution(n, size, counts, displs);

  int local_size = counts[rank];
  std::vector<int> local_data(local_size);

  int *sendbuf = (n > 0) ? all_data.data() : nullptr;
  MPI_Scatterv(sendbuf, counts.data(), displs.data(), MPI_INT, (local_size > 0) ? local_data.data() : nullptr,
               local_size, MPI_INT, 0, MPI_COMM_WORLD);

  if (local_size > 0) {
    std::sort(local_data.begin(), local_data.end());
  }

  std::vector<int> gathered_data;
  int *recvbuf = nullptr;

  if (rank == 0) {
    gathered_data.resize(n);
    recvbuf = gathered_data.data();
  }

  MPI_Gatherv((local_size > 0) ? local_data.data() : nullptr, local_size, MPI_INT, recvbuf, counts.data(),
              displs.data(), MPI_INT, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    if (size == 1) {
      GetOutput() = gathered_data;
    } else {
      std::vector<int> sorted_data;
      int first_non_empty = 0;

      while (first_non_empty < size && counts[first_non_empty] == 0) {
        first_non_empty++;
      }

      if (first_non_empty < size) {
        int start = displs[first_non_empty];
        int end = start + counts[first_non_empty];
        sorted_data.assign(gathered_data.begin() + start, gathered_data.begin() + end);

        for (int i = first_non_empty + 1; i < size; ++i) {
          if (counts[i] > 0) {
            int part_start = displs[i];
            int part_end = part_start + counts[i];
            std::vector<int> part(gathered_data.begin() + part_start, gathered_data.begin() + part_end);

            std::vector<int> merged;
            merged.reserve(sorted_data.size() + part.size());
            std::merge(sorted_data.begin(), sorted_data.end(), part.begin(), part.end(), std::back_inserter(merged));
            sorted_data = std::move(merged);
          }
        }
      }

      GetOutput() = sorted_data;
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

  size_t i = 0, j = 0;

  while (i < left.size() && j < right.size()) {
    if (left[i] < right[j]) {
      result.push_back(left[i++]);
    } else {
      result.push_back(right[j++]);
    }
  }

  while (i < left.size()) {
    result.push_back(left[i++]);
  }

  while (j < right.size()) {
    result.push_back(right[j++]);
  }

  return result;
}

std::vector<int> ShkrylevaSQSortSMergeMPI::QuickSortWithMerge(const std::vector<int> &arr) {
  std::vector<int> result = arr;
  std::sort(result.begin(), result.end());
  return result;
}

}  // namespace shkryleva_s_qsort_smerge
