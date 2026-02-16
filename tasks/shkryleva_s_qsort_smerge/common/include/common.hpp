#pragma once

#include <algorithm>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "task/include/task.hpp"

namespace shkryleva_s_qsort_smerge {

using InType = std::vector<int>;
using OutType = std::vector<int>;
using TestType = std::tuple<std::vector<int>, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

inline void SortThreeElements(std::vector<int> &arr, int left, int mid, int right) {
  if (arr[left] > arr[mid]) {
    std::swap(arr[left], arr[mid]);
  }
  if (arr[left] > arr[right]) {
    std::swap(arr[left], arr[right]);
  }
  if (arr[mid] > arr[right]) {
    std::swap(arr[mid], arr[right]);
  }
}

inline void QuickSortIterative(std::vector<int> &arr) {
  if (arr.size() <= 1) {
    return;
  }

  struct StackItem {
    int left;
    int right;
  };

  std::vector<StackItem> stack{{0, static_cast<int>(arr.size()) - 1}};

  while (!stack.empty()) {
    const auto [left, right] = stack.back();
    stack.pop_back();

    if (left >= right) {
      continue;
    }

    const int mid = left + ((right - left) / 2);

    SortThreeElements(arr, left, mid, right);
    std::swap(arr[mid], arr[right]);
    const int pivot = arr[right];

    int i = left - 1;
    for (int j = left; j < right; ++j) {
      if (arr[j] <= pivot) {
        ++i;
        std::swap(arr[i], arr[j]);
      }
    }

    std::swap(arr[i + 1], arr[right]);
    const int partition = i + 1;

    if (partition - left > right - partition) {
      stack.push_back({left, partition - 1});
      stack.push_back({partition + 1, right});
    } else {
      stack.push_back({partition + 1, right});
      stack.push_back({left, partition - 1});
    }
  }
}

}  // namespace shkryleva_s_qsort_smerge
