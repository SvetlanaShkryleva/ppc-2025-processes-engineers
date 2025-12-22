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

  MPI_Scatterv(rank == 0 ? GetInput().data() : nullptr, counts.data(), displs.data(), MPI_INT, local_data.data(),
               counts[rank], MPI_INT, 0, MPI_COMM_WORLD);

  if (!local_data.empty()) {
    std::sort(local_data.begin(), local_data.end());
  }

  std::vector<int> gathered_data;
  if (rank == 0) {
    gathered_data.resize(n);
  }

  MPI_Gatherv(local_data.data(), counts[rank], MPI_INT, rank == 0 ? gathered_data.data() : nullptr, counts.data(),
              displs.data(), MPI_INT, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    for (int i = 0; i < size; ++i) {
      if (counts[i] > 0) {
        for (int j = displs[i] + 1; j < displs[i] + counts[i]; ++j) {
          if (gathered_data[j] < gathered_data[j - 1]) {
            std::sort(gathered_data.begin(), gathered_data.end());
            break;
          }
        }
      }
    }

    MergeSortedParts(gathered_data, counts, displs, size);
    GetOutput() = gathered_data;
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

  std::vector<int> merged;
  if (counts[first_non_empty] > 0) {
    merged.assign(data.begin() + displs[first_non_empty],
                  data.begin() + displs[first_non_empty] + counts[first_non_empty]);
  }

  for (int i = first_non_empty + 1; i < size; ++i) {
    if (counts[i] > 0) {
      std::vector<int> current_part(data.begin() + displs[i], data.begin() + displs[i] + counts[i]);
      merged = MergeTwoSortedVectors(merged, current_part);
    }
  }

  int total_elements = 0;
  for (int i = 0; i < size; ++i) {
    total_elements += counts[i];
  }

  if (merged.size() != static_cast<size_t>(total_elements)) {
    std::sort(data.begin(), data.end());
    return;
  }

  std::copy(merged.begin(), merged.end(), data.begin());
}

}  // namespace shkryleva_s_qsort_smerge
