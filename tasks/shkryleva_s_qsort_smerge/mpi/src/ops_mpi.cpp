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
    is_valid = (!GetInput().empty()) ? 1 : 0;
  }
  MPI_Bcast(&is_valid, 1, MPI_INT, 0, MPI_COMM_WORLD);

  return is_valid != 0;
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

  // Получаем входной вектор
  std::vector<int> input;
  int n = 0;
  if (rank == 0) {
    input = GetInput();
    n = input.size();
  }

  // Рассылаем размер вектора
  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

  // Распределение данных по процессам
  std::vector<int> counts(size);
  std::vector<int> displs(size);
  ComputeDistribution(n, size, counts, displs);

  int local_size = counts[rank];
  std::vector<int> local_data(local_size, 0);

  // Разбрасываем данные (только если есть что разбрасывать)
  if (n > 0) {
    MPI_Scatterv(input.data(), counts.data(), displs.data(), MPI_INT, local_data.data(), local_size, MPI_INT, 0,
                 MPI_COMM_WORLD);
  }

  // Сортируем локальные данные
  local_data = QuickSortWithMerge(local_data);

  // Собираем отсортированные части
  std::vector<int> gathered_data;
  if (rank == 0) {
    gathered_data.resize(n);
  }

  if (n > 0) {
    MPI_Gatherv(local_data.data(), local_size, MPI_INT, gathered_data.data(), counts.data(), displs.data(), MPI_INT, 0,
                MPI_COMM_WORLD);
  }

  // В процессе 0 выполняем финальное слияние
  if (rank == 0 && n > 0) {
    std::vector<int> sorted_data = std::vector<int>(gathered_data.begin(), gathered_data.begin() + counts[0]);
    for (int i = 1; i < size; ++i) {
      std::vector<int> part(gathered_data.begin() + displs[i], gathered_data.begin() + displs[i] + counts[i]);
      sorted_data = Merge(sorted_data, part);
    }

    GetOutput() = sorted_data;
  } else if (rank == 0) {
    GetOutput() = std::vector<int>();
  }

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
  if (arr.size() <= 1) {
    return arr;
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
