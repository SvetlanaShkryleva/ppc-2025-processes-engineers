#include "shkryleva_s_qsort_smerge/seq/include/ops_seq.hpp"

#include <span>
#include <vector>

#include "shkryleva_s_qsort_smerge/common/include/common.hpp"

namespace shkryleva_s_qsort_smerge {

ShkrylevaSQSortSMergeSEQ::ShkrylevaSQSortSMergeSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = std::vector<int>();
}

bool ShkrylevaSQSortSMergeSEQ::ValidationImpl() {
  return GetOutput().empty();
}

bool ShkrylevaSQSortSMergeSEQ::PreProcessingImpl() {
  return true;
}

std::vector<int> ShkrylevaSQSortSMergeSEQ::Merge(const std::vector<int> &left, const std::vector<int> &right) {
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

std::vector<int> ShkrylevaSQSortSMergeSEQ::QuickSortWithMerge(const std::span<int> &arr) {
  if (arr.empty()) {
    return std::vector<int>();
  }

  if (arr.size() == 1) {
    std::vector<int> res;
    res.assign(arr.begin(), arr.end());
    return res;
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
