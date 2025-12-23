#include "shkryleva_s_qsort_smerge/seq/include/ops_seq.hpp"

#include <algorithm>
#include <functional>
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

static void QuickSortHelper(std::vector<int> &arr, int left, int right) {
  if (left >= right) {
    return;
  }

  int mid = left + ((right - left) / 2);

  if (arr[right] < arr[left]) {
    std::swap(arr[left], arr[right]);
  }
  if (arr[mid] < arr[left]) {
    std::swap(arr[mid], arr[left]);
  }
  if (arr[right] < arr[mid]) {
    std::swap(arr[right], arr[mid]);
  }

  int pivot = arr[mid];
  int i = left;
  int j = right;

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

  QuickSortHelper(arr, left, j);
  QuickSortHelper(arr, i, right);
}

std::vector<int> ShkrylevaSQSortSMergeSEQ::QuickSortWithMerge(const std::vector<int> &arr) {
  if (arr.empty()) {
    return {};
  }

  if (arr.size() == 1) {
    return {arr[0]};
  }

  std::vector<int> result;
  result.reserve(arr.size());
  for (int value : arr) {
    result.push_back(value);
  }

  QuickSortHelper(result, 0, static_cast<int>(result.size()) - 1);
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
