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

  std::function<void(int, int)> quick_sort;

  quick_sort = [&result, &quick_sort](int left, int right) {
    if (left >= right) {
      return;
    }

    int mid = left + (right - left) / 2;

    if (result[right] < result[left]) {
      std::swap(result[left], result[right]);
    }
    if (result[mid] < result[left]) {
      std::swap(result[mid], result[left]);
    }
    if (result[right] < result[mid]) {
      std::swap(result[right], result[mid]);
    }

    int pivot = result[mid];
    int i = left, j = right;

    while (i <= j) {
      while (result[i] < pivot) {
        i++;
      }
      while (result[j] > pivot) {
        j--;
      }
      if (i <= j) {
        std::swap(result[i], result[j]);
        i++;
        j--;
      }
    }

    quick_sort(left, j);
    quick_sort(i, right);
  };

  quick_sort(0, static_cast<int>(result.size()) - 1);
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
