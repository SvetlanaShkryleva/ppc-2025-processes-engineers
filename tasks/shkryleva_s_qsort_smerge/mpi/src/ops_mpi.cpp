#include "shkryleva_s_qsort_smerge/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cstddef>
#include <numeric>
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
  GetOutput() = GetInput();
  return true;
}

bool ShkrylevaSQSortSMergeMPI::RunImpl() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (GetOutput().empty()) {
    MPI_Barrier(MPI_COMM_WORLD);
    return true;
  }

  int n = static_cast<int>(GetOutput().size());

  std::vector<int> counts(size);
  std::vector<int> displs(size);
  ComputeDistribution(n, size, counts, displs);

  std::vector<int> local_data(counts[rank]);

  if (rank == 0) {
    MPI_Scatterv(GetOutput().data(), counts.data(), displs.data(), MPI_INT, local_data.data(), counts[rank], MPI_INT, 0,
                 MPI_COMM_WORLD);
  } else {
    MPI_Scatterv(nullptr, counts.data(), displs.data(), MPI_INT, local_data.data(), counts[rank], MPI_INT, 0,
                 MPI_COMM_WORLD);
  }

  if (!local_data.empty()) {
    std::ranges::sort(local_data);
  }

  if (rank == 0) {
    std::vector<int> result = local_data;

    for (int i = 1; i < size; ++i) {
      if (counts[i] > 0) {
        std::vector<int> received_data(counts[i]);
        MPI_Recv(received_data.data(), counts[i], MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        result = MergeTwoSortedVectors(result, received_data);
      }
    }

    GetOutput() = std::move(result);

    for (int i = 1; i < size; ++i) {
      MPI_Send(GetOutput().data(), static_cast<int>(GetOutput().size()), MPI_INT, i, 1, MPI_COMM_WORLD);
    }
  } else {
    if (!local_data.empty()) {
      MPI_Send(local_data.data(), static_cast<int>(local_data.size()), MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    int recv_size = 0;
    MPI_Status status;

    MPI_Probe(0, 1, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_INT, &recv_size);

    GetOutput().resize(recv_size);
    MPI_Recv(GetOutput().data(), recv_size, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }

  MPI_Barrier(MPI_COMM_WORLD);
  return true;
}

bool ShkrylevaSQSortSMergeMPI::PostProcessingImpl() {
  if (GetOutput().empty()) {
    return GetInput().empty();
  }

  if (!std::ranges::is_sorted(GetOutput())) {
    return false;
  }

  if (GetOutput().size() != GetInput().size()) {
    return false;
  }

  int sum_input = std::accumulate(GetInput().begin(), GetInput().end(), 0);
  int sum_output = std::accumulate(GetOutput().begin(), GetOutput().end(), 0);

  return sum_input == sum_output;
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

  size_t i = 0;
  size_t j = 0;

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

}  // namespace shkryleva_s_qsort_smerge
