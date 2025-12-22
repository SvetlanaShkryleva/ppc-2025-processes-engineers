#include "shkryleva_s_qsort_smerge/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cstddef>
#include <utility>
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

  int n = 0;
  if (rank == 0) {
    n = static_cast<int>(GetInput().size());
  }
  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (n == 0) {
    GetOutput() = std::vector<int>();
    return true;
  }

  std::vector<int> counts(size);
  std::vector<int> displs(size);
  ComputeDistribution(n, size, counts, displs);

  std::vector<int> local_data(counts[rank]);

  if (rank == 0) {
    std::vector<int> all_data = GetInput();

    for (int i = 0; i < size; ++i) {
      if (counts[i] > 0) {
        if (i == 0) {
          std::copy(all_data.begin() + displs[i], all_data.begin() + displs[i] + counts[i], local_data.begin());
        } else {
          MPI_Send(all_data.data() + displs[i], counts[i], MPI_INT, i, 0, MPI_COMM_WORLD);
        }
      }
    }
  } else {
    if (counts[rank] > 0) {
      MPI_Recv(local_data.data(), counts[rank], MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
  }

  if (counts[rank] > 0) {
    std::sort(local_data.begin(), local_data.end());
  }

  std::vector<int> sorted_data;
  if (rank == 0) {
    sorted_data.resize(n);

    if (counts[0] > 0) {
      std::copy(local_data.begin(), local_data.begin() + counts[0],
                sorted_data.begin() + displs[0]);  // <-- ИСПРАВЛЕНО!
    }

    for (int i = 1; i < size; ++i) {
      if (counts[i] > 0) {
        MPI_Recv(sorted_data.data() + displs[i], counts[i], MPI_INT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      }
    }

    MergeSortedParts(sorted_data, counts, displs, size);
    GetOutput() = sorted_data;
  } else {
    if (counts[rank] > 0) {
      MPI_Send(local_data.data(), counts[rank], MPI_INT, 0, 1, MPI_COMM_WORLD);
    }
  }

  int output_size = 0;
  if (rank == 0) {
    output_size = static_cast<int>(GetOutput().size());
  }
  MPI_Bcast(&output_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (rank != 0) {
    GetOutput().resize(output_size);
  }

  MPI_Bcast(GetOutput().data(), output_size, MPI_INT, 0, MPI_COMM_WORLD);

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

std::vector<int> ShkrylevaSQSortSMergeMPI::MergeTwoSortedVectors(const std::vector<int> &a, const std::vector<int> &b) {
  std::vector<int> result;
  result.reserve(a.size() + b.size());

  size_t i = 0, j = 0;
  while (i < a.size() && j < b.size()) {
    if (a[i] <= b[j]) {
      result.push_back(a[i++]);
    } else {
      result.push_back(b[j++]);
    }
  }

  while (i < a.size()) {
    result.push_back(a[i++]);
  }

  while (j < b.size()) {
    result.push_back(b[j++]);
  }

  return result;
}

void ShkrylevaSQSortSMergeMPI::MergeSortedParts(std::vector<int> &data, const std::vector<int> &counts,
                                                const std::vector<int> &displs, int size) {
  if (size == 1) {
    return;
  }

  int first_non_empty = 0;
  while (first_non_empty < size && counts[first_non_empty] == 0) {
    first_non_empty++;
  }

  if (first_non_empty >= size) {
    return;
  }

  std::vector<int> merged_part;
  merged_part.assign(data.begin() + displs[first_non_empty],
                     data.begin() + displs[first_non_empty] + counts[first_non_empty]);

  for (int i = first_non_empty + 1; i < size; ++i) {
    if (counts[i] > 0) {
      std::vector<int> current_part(data.begin() + displs[i], data.begin() + displs[i] + counts[i]);
      merged_part = MergeTwoSortedVectors(merged_part, current_part);
    }
  }

  int total_elements = 0;
  for (int i = 0; i < size; ++i) {
    total_elements += counts[i];
  }

  if (merged_part.size() != static_cast<size_t>(total_elements)) {
    std::sort(data.begin(), data.end());
    return;
  }

  std::copy(merged_part.begin(), merged_part.end(), data.begin());
}

}  // namespace shkryleva_s_qsort_smerge
