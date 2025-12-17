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
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int is_valid = 0;
  if (rank == 0) {
    is_valid = 1;
  }

  MPI_Bcast(&is_valid, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (is_valid == 0) {
    int mpi_initialized;
    MPI_Initialized(&mpi_initialized);
    if (mpi_initialized) {
      MPI_Finalize();
    }
    return false;
  }

  return true;
}
bool ShkrylevaSQSortSMergeMPI::PreProcessingImpl() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  GetOutput() = std::vector<int>();

  MPI_Barrier(MPI_COMM_WORLD);
  return true;
}
bool ShkrylevaSQSortSMergeMPI::RunImpl() {
  int rank = 0;
  int size = 1;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  std::vector<int> input;
  int n = 0;
  if (rank == 0) {
    input = GetInput();
    n = input.size();
  }

  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (n == 0) {
    if (rank == 0) {
      GetOutput() = std::vector<int>();
    }
    return true;
  }

  std::vector<int> counts(size);
  std::vector<int> displs(size);
  ComputeDistribution(n, size, counts, displs);

  int local_size = counts[rank];
  std::vector<int> local_data;

  if (local_size > 0) {
    local_data.resize(local_size, 0);
  }

  int *sendbuf = (rank == 0) ? input.data() : nullptr;
  int *recvbuf = (local_size > 0) ? local_data.data() : nullptr;

  MPI_Scatterv(sendbuf, counts.data(), displs.data(), MPI_INT, recvbuf, local_size, MPI_INT, 0, MPI_COMM_WORLD);

  if (local_size > 0) {
    local_data = QuickSortWithMerge(local_data);
  }

  std::vector<int> gathered_data;
  int *gathered_buf = nullptr;

  if (rank == 0) {
    gathered_data.resize(n);
    gathered_buf = gathered_data.data();
  }

  int *sendbuf_gather = (local_size > 0) ? local_data.data() : nullptr;
  MPI_Gatherv(sendbuf_gather, local_size, MPI_INT, gathered_buf, counts.data(), displs.data(), MPI_INT, 0,
              MPI_COMM_WORLD);

  if (rank == 0) {
    int first_proc_with_data = 0;
    while (first_proc_with_data < size && counts[first_proc_with_data] == 0) {
      first_proc_with_data++;
    }

    std::vector<int> sorted_data;

    if (first_proc_with_data < size) {
      sorted_data =
          std::vector<int>(gathered_data.begin() + displs[first_proc_with_data],
                           gathered_data.begin() + displs[first_proc_with_data] + counts[first_proc_with_data]);

      for (int i = first_proc_with_data + 1; i < size; ++i) {
        if (counts[i] > 0) {
          std::vector<int> part(gathered_data.begin() + displs[i], gathered_data.begin() + displs[i] + counts[i]);
          sorted_data = Merge(sorted_data, part);
        }
      }
    } else {
      sorted_data = std::vector<int>();
    }

    GetOutput() = sorted_data;
  }

  MPI_Barrier(MPI_COMM_WORLD);
  return true;
}

bool ShkrylevaSQSortSMergeMPI::PostProcessingImpl() {
  return true;
}

void ShkrylevaSQSortSMergeMPI::ComputeDistribution(int n, int size, std::vector<int> &counts,
                                                   std::vector<int> &displs) {
  int offset = 0;
  for (int proc = 0; proc < size; ++proc) {
    int base = n / size;
    int extra = (proc < (n % size)) ? 1 : 0;
    int proc_size = base + extra;

    counts[proc] = proc_size;
    displs[proc] = offset;
    offset += proc_size;
  }
}

std::vector<int> ShkrylevaSQSortSMergeMPI::Merge(const std::vector<int> &left, const std::vector<int> &right) {
  std::vector<int> result;
  result.reserve(left.size() + right.size());

  size_t i = 0;
  size_t j = 0;

  while (i < left.size() && j < right.size()) {
    if (left[i] < right[j]) {
      result.push_back(left[i++]);
    } else {
      result.push_back(right[j++]);
    }
  }

  result.insert(result.end(), left.begin() + i, left.end());
  result.insert(result.end(), right.begin() + j, right.end());

  return result;
}
std::vector<int> ShkrylevaSQSortSMergeMPI::QuickSortWithMerge(const std::vector<int> &arr) {
  if (arr.empty()) {
    return std::vector<int>();
  }

  if (arr.size() == 1) {
    return std::vector<int>{arr[0]};
  }

  int pivot = arr[arr.size() / 2];
  std::vector<int> left;
  std::vector<int> right;
  std::vector<int> equal;

  for (const auto &elem : arr) {
    if (elem < pivot) {
      left.emplace_back(elem);
    } else if (elem > pivot) {
      right.emplace_back(elem);
    } else {
      equal.emplace_back(elem);
    }
  }

  std::vector<int> sortedLeft = QuickSortWithMerge(left);
  std::vector<int> sortedRight = QuickSortWithMerge(right);

  std::vector<int> merged = Merge(sortedLeft, equal);
  return Merge(merged, sortedRight);
}

}  // namespace shkryleva_s_qsort_smerge
