#include "shkryleva_s_qsort_smerge/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <vector>

namespace shkryleva_s_qsort_smerge {

ShkrylevaSQSortSMergeMPI::ShkrylevaSQSortSMergeMPI(const InType& inputVector) {
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

void quickSort(std::vector<int>& arr, int left, int right) {
  if (left >= right) return;

  int pivot = arr[(left + right) / 2];
  int i = left;
  int j = right;

  while (i <= j) {
    while (arr[i] < pivot) ++i;
    while (arr[j] > pivot) --j;

    if (i <= j) {
      std::swap(arr[i], arr[j]);
      ++i;
      --j;
    }
  }

  if (left < j) quickSort(arr, left, j);
  if (i < right) quickSort(arr, i, right);
}

std::vector<int> mergeArrays(const std::vector<int>& a,
                             const std::vector<int>& b) {
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

  while (i < a.size()) result.push_back(a[i++]);
  while (j < b.size()) result.push_back(b[j++]);

  return result;
}

}  // namespace

bool ShkrylevaSQSortSMergeMPI::RunImpl() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int total_size = 0;
  if (rank == 0) {
    total_size = static_cast<int>(GetOutput().size());
  }

  MPI_Bcast(&total_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (total_size == 0) {
    return true;
  }

  std::vector<int> sendcounts(size);
  std::vector<int> displs(size);

  int base = total_size / size;
  int rem = total_size % size;

  for (int i = 0; i < size; ++i) {
    sendcounts[i] = base + (i < rem ? 1 : 0);
    displs[i] = (i == 0 ? 0 : displs[i - 1] + sendcounts[i - 1]);
  }

  std::vector<int> local(sendcounts[rank]);

  MPI_Scatterv(rank == 0 ? GetOutput().data() : nullptr,
               sendcounts.data(),
               displs.data(),
               MPI_INT,
               local.data(),
               sendcounts[rank],
               MPI_INT,
               0,
               MPI_COMM_WORLD);

  if (!local.empty()) {
    quickSort(local, 0, static_cast<int>(local.size()) - 1);
  }

  if (rank == 0) {
    std::vector<int> result = local;

    for (int i = 1; i < size; ++i) {
      std::vector<int> recvbuf(sendcounts[i]);
      MPI_Recv(recvbuf.data(),
               sendcounts[i],
               MPI_INT,
               i,
               0,
               MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);

      result = mergeArrays(result, recvbuf);
    }

    GetOutput() = result;
  } else {
    MPI_Send(local.data(),
             sendcounts[rank],
             MPI_INT,
             0,
             0,
             MPI_COMM_WORLD);
  }

  return true;
}

bool ShkrylevaSQSortSMergeMPI::PostProcessingImpl() {
  return true;
}

}  // namespace shkryleva_s_qsort_smerge
