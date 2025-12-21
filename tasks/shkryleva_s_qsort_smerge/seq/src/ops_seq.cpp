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

std::vector<int> ShkrylevaSQSortSMergeSEQ::QuickSort(const std::vector<int> &arr) {
  if (arr.size() <= 1) {
    return arr;
  }

  int pivot = arr[arr.size() / 2];

  std::vector<int> less, equal, greater;

  for (const auto &elem : arr) {
    if (elem < pivot) {
      less.push_back(elem);
    } else if (elem > pivot) {
      greater.push_back(elem);
    } else {
      equal.push_back(elem);
    }
  }

  std::vector<int> sorted_less = QuickSort(less);
  std::vector<int> sorted_greater = QuickSort(greater);

  std::vector<int> result;
  result.reserve(sorted_less.size() + equal.size() + sorted_greater.size());

  result.insert(result.end(), sorted_less.begin(), sorted_less.end());
  result.insert(result.end(), equal.begin(), equal.end());
  result.insert(result.end(), sorted_greater.begin(), sorted_greater.end());

  return result;
}

bool ShkrylevaSQSortSMergeSEQ::RunImpl() {
  if (GetInput().empty()) {
    GetOutput() = std::vector<int>();
    return true;
  }

  std::vector<int> sorted = QuickSort(GetInput());

  for (size_t i = 1; i < sorted.size(); i++) {
    if (sorted[i] < sorted[i - 1]) {
      sorted = GetInput();
      std::sort(sorted.begin(), sorted.end());
      break;
    }
  }

  GetOutput() = sorted;
  return true;
}

bool ShkrylevaSQSortSMergeSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace shkryleva_s_qsort_smerge
