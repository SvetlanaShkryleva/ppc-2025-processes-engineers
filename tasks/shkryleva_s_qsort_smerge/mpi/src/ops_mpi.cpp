#include "shkryleva_s_qsort_smerge/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <vector>

namespace shkryleva_s_qsort_smerge {

ShkrylevaSQSortSMergeMPI::ShkrylevaSQSortSMergeMPI(const InType &inputVector) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = inputVector;
  GetOutput() = {};
}

bool ShkrylevaSQSortSMergeMPI::ValidationImpl() {
  return true;
}

bool ShkrylevaSQSortSMergeMPI::PreProcessingImpl() {
  GetOutput() = GetInput();
  return true;
}

namespace {

void quickSort(std::vector<int> &arr, int left, int right) {
  if (left >= right) {
    return;
  }

  int pivot = arr[(left + right) / 2];
  int i = left;
  int j = right;

  while (i <= j) {
    while (arr[i] < pivot) {
      ++i;
    }
    while (arr[j] > pivot) {
      --j;
    }

    if (i <= j) {
      std::swap(arr[i], arr[j]);
      ++i;
      --j;
    }
  }

  if (left < j) {
    quickSort(arr, left, j);
  }
  if (i < right) {
    quickSort(arr, i, right);
  }
}

}  // namespace

bool ShkrylevaSQSortSMergeMPI::RunImpl() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int total_size = 0;
  std::vector<int> input;

  if (rank == 0) {
    input = GetOutput();
    total_size = static_cast<int>(input.size());
  }

  MPI_Bcast(&total_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  std::vector<int> sendcounts(size);
  std::vector<int> displs(size);

  int base = total_size / size;
  int rem = total_size % size;

  for (int i = 0; i < size; ++i) {
    sendcounts[i] = base + (i < rem ? 1 : 0);
    displs[i] = (i == 0 ? 0 : displs[i - 1] + sendcounts[i - 1]);
  }

  std::vector<int> local(sendcounts[rank]);

  MPI_Scatterv(rank == 0 ? input.data() : nullptr, sendcounts.data(), displs.data(), MPI_INT,
               sendcounts[rank] > 0 ? local.data() : nullptr, sendcounts[rank], MPI_INT, 0, MPI_COMM_WORLD);

  if (!local.empty()) {
    std::sort(local.begin(), local.end());
  }

  std::vector<int> gathered;
  if (rank == 0) {
    gathered.resize(total_size);
  }

  MPI_Gatherv(sendcounts[rank] > 0 ? local.data() : nullptr, sendcounts[rank], MPI_INT,
              rank == 0 ? gathered.data() : nullptr, sendcounts.data(), displs.data(), MPI_INT, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    std::sort(gathered.begin(), gathered.end());
    GetOutput() = gathered;
  }

  return true;
}

bool ShkrylevaSQSortSMergeMPI::PostProcessingImpl() {
  return true;
}

}  // namespace shkryleva_s_qsort_smerge
