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
    if (rank == 0) {
      GetOutput() = std::vector<int>();
    }
    return true;
  }

  std::vector<int> counts(size);
  std::vector<int> displs(size);
  ComputeDistribution(n, size, counts, displs);

  std::vector<int> local_data(counts[rank]);

  if (rank == 0) {
    MPI_Scatterv(GetInput().data(), counts.data(), displs.data(), MPI_INT,
                 MPI_IN_PLACE, 0, MPI_INT, 0, MPI_COMM_WORLD);
  } else {
    MPI_Scatterv(nullptr, counts.data(), displs.data(), MPI_INT,
                 local_data.data(), counts[rank], MPI_INT, 0, MPI_COMM_WORLD);
  }

  if (rank == 0) {
    local_data = std::vector<int>(GetInput().begin(), GetInput().begin() + counts[0]);
  }

  if (!local_data.empty()) {
    std::sort(local_data.begin(), local_data.end());
  }

  std::vector<int> gathered_data;
  if (rank == 0) {
    gathered_data.resize(n);
  }

  int local_size = static_cast<int>(local_data.size());
  std::vector<int> all_sizes(size);
  MPI_Gather(&local_size, 1, MPI_INT, all_sizes.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

  std::vector<int> recv_displs(size, 0);
  if (rank == 0) {
    for (int i = 1; i < size; ++i) {
      recv_displs[i] = recv_displs[i - 1] + all_sizes[i - 1];
    }
  }

  MPI_Gatherv(local_data.data(), local_size, MPI_INT,
              rank == 0 ? gathered_data.data() : nullptr,
              all_sizes.data(), recv_displs.data(), MPI_INT, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    for (int i = 0; i < size; ++i) {
      if (all_sizes[i] > 0) {
        int start_idx = recv_displs[i];
        for (int j = start_idx + 1; j < start_idx + all_sizes[i]; ++j) {
          if (gathered_data[j] < gathered_data[j - 1]) {
            std::sort(gathered_data.begin(), gathered_data.end());
            break;
          }
        }
      }
    }

    if (size > 1) {
      MergeSortedParts(gathered_data, all_sizes, recv_displs, size);
    }

    if (!std::is_sorted(gathered_data.begin(), gathered_data.end())) {
      std::sort(gathered_data.begin(), gathered_data.end());
    }

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

  std::vector<std::vector<int>> parts;
  std::vector<int> valid_displs;
  
  for (int i = 0; i < size; ++i) {
    if (counts[i] > 0) {
      parts.push_back(std::vector<int>(
          data.begin() + displs[i],
          data.begin() + displs[i] + counts[i]
      ));
      valid_displs.push_back(displs[i]);
    }
  }

  if (parts.empty()) {
    return;
  }

  std::vector<int> merged = parts[0];
  for (size_t i = 1; i < parts.size(); ++i) {
    merged = MergeTwoSortedVectors(merged, parts[i]);
  }

  std::copy(merged.begin(), merged.end(), data.begin());
}

}  // namespace shkryleva_s_qsort_smerge