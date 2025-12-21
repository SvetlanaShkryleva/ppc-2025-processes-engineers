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

// Реализация быстрой сортировки с использованием итераторов
template <typename RandomIt>
void quick_sort_impl(RandomIt first, RandomIt last) {
  if (first == last || std::next(first) == last) {
    return;
  }

  auto pivot = *std::next(first, std::distance(first, last) / 2);
  RandomIt middle1 = std::partition(first, last, [pivot](const auto &em) { return em < pivot; });
  RandomIt middle2 = std::partition(middle1, last, [pivot](const auto &em) { return !(pivot < em); });

  quick_sort_impl(first, middle1);
  quick_sort_impl(middle2, last);
}

std::vector<int> ShkrylevaSQSortSMergeSEQ::QuickSortWithMerge(const std::vector<int> &arr) {
  if (arr.empty()) {
    return arr;
  }

  std::vector<int> result = arr;
  quick_sort_impl(result.begin(), result.end());
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
