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
  return true;
}

bool ShkrylevaSQSortSMergeMPI::RunImpl() {
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int n = 0;
  if (rank == 0) {
    n = static_cast<int>(GetInput().size());
  }
  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (n == 0) {
    GetOutput() = std::vector<int>();
    MPI_Barrier(MPI_COMM_WORLD);
    return true;
  }

  std::vector<int> all_data(n);
  if (rank == 0) {
    all_data = GetInput();
  }
  MPI_Bcast(all_data.data(), n, MPI_INT, 0, MPI_COMM_WORLD);

  std::vector<int> counts(size), displs(size);
  ComputeDistribution(n, size, counts, displs);

  std::vector<int> local_data(counts[rank]);
  if (counts[rank] > 0) {
    std::copy(all_data.begin() + displs[rank], all_data.begin() + displs[rank] + counts[rank], local_data.begin());
  }

  if (counts[rank] > 0) {
    std::sort(local_data.begin(), local_data.end());
  }

  std::vector<int> gathered_data;
  if (rank == 0) {
    gathered_data.resize(n);
  }

  MPI_Gatherv(local_data.data(), counts[rank], MPI_INT, (rank == 0) ? gathered_data.data() : nullptr, counts.data(),
              displs.data(), MPI_INT, 0, MPI_COMM_WORLD);

  std::vector<int> sorted_data;
  if (rank == 0) {
    if (size == 1) {
      sorted_data = gathered_data;
    } else {
      sorted_data.assign(gathered_data.begin(), gathered_data.begin() + counts[0]);

      for (int i = 1; i < size; ++i) {
        if (counts[i] > 0) {
          std::vector<int> part(gathered_data.begin() + displs[i], gathered_data.begin() + displs[i] + counts[i]);
          std::vector<int> merged(sorted_data.size() + part.size());
          std::merge(sorted_data.begin(), sorted_data.end(), part.begin(), part.end(), merged.begin());
          sorted_data = std::move(merged);
        }
      }
    }
  }

  int sorted_size = 0;
  if (rank == 0) {
    sorted_size = static_cast<int>(sorted_data.size());
  }
  MPI_Bcast(&sorted_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (rank != 0) {
    sorted_data.resize(sorted_size);
  }

  MPI_Bcast(sorted_data.data(), sorted_size, MPI_INT, 0, MPI_COMM_WORLD);

  GetOutput() = sorted_data;

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
