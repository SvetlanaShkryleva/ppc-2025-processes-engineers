#include "shkryleva_s_qsort_smerge/seq/include/ops_seq.hpp"

#include <algorithm>
#include <vector>

namespace shkryleva_s_qsort_smerge {

ShkrylevaSQSortSMergeSEQ::ShkrylevaSQSortSMergeSEQ(const InType& inputData) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = inputData;
  GetOutput() = {};
}

bool ShkrylevaSQSortSMergeSEQ::ValidationImpl() {
  return true;
}

bool ShkrylevaSQSortSMergeSEQ::PreProcessingImpl() {
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

}  // namespace

bool ShkrylevaSQSortSMergeSEQ::RunImpl() {
  if (!GetOutput().empty()) {
    quickSort(GetOutput(), 0, static_cast<int>(GetOutput().size()) - 1);
  }
  return true;
}

bool ShkrylevaSQSortSMergeSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace shkryleva_s_qsort_smerge
