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

  std::vector<int> input = GetInput();
  int n = static_cast<int>(input.size());

  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (n == 0) {
    GetOutput() = std::vector<int>();
    MPI_Barrier(MPI_COMM_WORLD);
    return true;
  }

  std::vector<int> all_data;

  std::vector<int> all_sizes(size);
  MPI_Allgather(&n, 1, MPI_INT, all_sizes.data(), 1, MPI_INT, MPI_COMM_WORLD);

  int total_elements = 0;
  for (int sz : all_sizes) {
    total_elements += sz;
  }

  if (rank == 0) {
    all_data.resize(total_elements);
  }

  std::vector<int> displs(size);
  int offset = 0;
  for (int i = 0; i < size; ++i) {
    displs[i] = offset;
    offset += all_sizes[i];
  }

  MPI_Gatherv(input.data(), n, MPI_INT, (rank == 0) ? all_data.data() : nullptr, all_sizes.data(), displs.data(),
              MPI_INT, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    std::sort(all_data.begin(), all_data.end());
    GetOutput() = all_data;
  } else {
    GetOutput() = std::vector<int>();
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
