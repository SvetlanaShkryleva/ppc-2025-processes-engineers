#include "shkryleva_s_qsort_smerge/seq/include/ops_seq.hpp"

#include <algorithm>
#include <vector>

#include "shkryleva_s_qsort_smerge/common/include/common.hpp"

namespace shkryleva_s_qsort_smerge {

ShkrylevaSQSortSMergeSEQ::ShkrylevaSQSortSMergeSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = std::vector<int>();
}

bool ShkrylevaSQSortSMergeSEQ::ValidationImpl() {
  return true;
}

bool ShkrylevaSQSortSMergeSEQ::PreProcessingImpl() {
  return true;
}

void quick_sort_impl(std::vector<int> &arr, int left, int right) {
  if (left >= right) {
    return;
  }

  int pivot = arr[left + (right - left) / 2];
  int i = left, j = right;

  while (i <= j) {
    while (arr[i] < pivot) {
      i++;
    }
    while (arr[j] > pivot) {
      j--;
    }
    if (i <= j) {
      std::swap(arr[i], arr[j]);
      i++;
      j--;
    }
  }

  quick_sort_impl(arr, left, j);
  quick_sort_impl(arr, i, right);
}

std::vector<int> ShkrylevaSQSortSMergeSEQ::QuickSortWithMerge(const std::vector<int> &arr) {
  if (arr.empty()) {
    return arr;
  }

  std::vector<int> result = arr;
  quick_sort_impl(result, 0, static_cast<int>(result.size()) - 1);
  return result;
}

bool ShkrylevaSQSortSMergeSEQ::RunImpl() {
  if (GetInput().empty()) {
    GetOutput() = std::vector<int>();
    return true;
  }

  std::vector<int> sorted = QuickSortWithMerge(GetInput());

  GetOutput() = sorted;
  return true;
}

bool ShkrylevaSQSortSMergeSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace shkryleva_s_qsort_smerge
