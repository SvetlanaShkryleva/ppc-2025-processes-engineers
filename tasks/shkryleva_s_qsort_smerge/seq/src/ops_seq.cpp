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

std::vector<int> ShkrylevaSQSortSMergeSEQ::QuickSortWithMerge(const std::vector<int> &arr) {
  if (arr.size() <= 1) {
    return arr;
  }

  std::vector<int> result = arr;

  std::function<void(int, int)> quick_sort = [&](int low, int high) {
    if (low < high) {
      int pivot = result[(low + high) / 2];
      int i = low, j = high;

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

      quick_sort(low, j);
      quick_sort(i, high);
    }
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
