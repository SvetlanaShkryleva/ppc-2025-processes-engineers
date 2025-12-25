#include "shkryleva_s_qsort_smerge/seq/include/ops_seq.hpp"

#include <algorithm>
#include <cstddef>
#include <vector>

#include "shkryleva_s_qsort_smerge/common/include/common.hpp"

namespace shkryleva_s_qsort_smerge {

ShkrylevaSQSortSMergeSEQ::ShkrylevaSQSortSMergeSEQ(const InType &inputData) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = inputData;
  GetOutput() = {};
}

bool ShkrylevaSQSortSMergeSEQ::ValidationImpl() {
  if (!GetOutput().empty()) {
    return false;
  }
  if (GetInput().size() > 1000000) {
    return false;
  }
  return checkElementRange(GetInput());
}

bool ShkrylevaSQSortSMergeSEQ::PreProcessingImpl() {
  GetOutput() = GetInput();
  return verifyDataCopy();
}

namespace {

bool validateValueRange(int number) {
  const int MIN_LIMIT = -1000000;
  const int MAX_LIMIT = 1000000;
  return number >= MIN_LIMIT && number <= MAX_LIMIT;
}

bool ShkrylevaSQSortSMergeSEQ::checkElementRange(const std::vector<int> &data) {
  return std::all_of(data.begin(), data.end(), validateValueRange);
}

bool ShkrylevaSQSortSMergeSEQ::verifyDataCopy() {
  if (GetOutput().size() != GetInput().size()) {
    return false;
  }
  for (size_t index = 0; index < GetOutput().size(); ++index) {
    if (GetOutput()[index] != GetInput()[index]) {
      return false;
    }
  }
  return true;
}

// NOLINTNEXTLINE(misc-no-recursion)
void performQuickSort(std::vector<int> &dataArray, int startIdx, int endIdx) {
  if (startIdx >= endIdx) {
    return;
  }
  int middleValue = dataArray[(startIdx + endIdx) / 2];
  int leftPointer = startIdx;
  int rightPointer = endIdx;

  while (leftPointer <= rightPointer) {
    while (dataArray[leftPointer] < middleValue) {
      ++leftPointer;
    }
    while (dataArray[rightPointer] > middleValue) {
      --rightPointer;
    }
    if (leftPointer <= rightPointer) {
      std::swap(dataArray[leftPointer], dataArray[rightPointer]);
      ++leftPointer;
      --rightPointer;
    }
  }
  performQuickSort(dataArray, startIdx, rightPointer);
  performQuickSort(dataArray, leftPointer, endIdx);
}

std::vector<int> combineSortedArrays(const std::vector<int> &firstPart, const std::vector<int> &secondPart) {
  std::vector<int> mergedResult;
  mergedResult.reserve(firstPart.size() + secondPart.size());
  size_t firstIdx = 0;
  size_t secondIdx = 0;

  while (firstIdx < firstPart.size() && secondIdx < secondPart.size()) {
    if (firstPart[firstIdx] <= secondPart[secondIdx]) {
      mergedResult.push_back(firstPart[firstIdx]);
      ++firstIdx;
    } else {
      mergedResult.push_back(secondPart[secondIdx]);
      ++secondIdx;
    }
  }

  while (firstIdx < firstPart.size()) {
    mergedResult.push_back(firstPart[firstIdx]);
    ++firstIdx;
  }

  while (secondIdx < secondPart.size()) {
    mergedResult.push_back(secondPart[secondIdx]);
    ++secondIdx;
  }
  return mergedResult;
}

bool sortArrayParts(std::vector<int> &data) {
  size_t dataSize = data.size();
  if (dataSize <= 1) {
    return true;
  }

  size_t midpoint = dataSize / 2;
  std::vector<int> leftSection(data.begin(), data.begin() + static_cast<ptrdiff_t>(midpoint));
  std::vector<int> rightSection(data.begin() + static_cast<ptrdiff_t>(midpoint), data.end());

  performQuickSort(leftSection, 0, static_cast<int>(leftSection.size()) - 1);
  performQuickSort(rightSection, 0, static_cast<int>(rightSection.size()) - 1);

  data = combineSortedArrays(leftSection, rightSection);
  return true;
}

}  // namespace

bool ShkrylevaSQSortSMergeSEQ::RunImpl() {
  if (GetOutput().empty()) {
    return true;
  }
  return sortArrayParts(GetOutput());
}

bool checkDataConsistency(const std::vector<int> &original, const std::vector<int> &processed) {
  if (original.empty()) {
    return processed.empty();
  }

  if (!std::is_sorted(processed.begin(), processed.end())) {
    return false;
  }

  if (original.size() != processed.size()) {
    return false;
  }

  int originalSum = 0;
  int processedSum = 0;
  for (const auto &value : original) {
    originalSum += value;
  }
  for (const auto &value : processed) {
    processedSum += value;
  }
  return originalSum == processedSum;
}

bool ShkrylevaSQSortSMergeSEQ::PostProcessingImpl() {
  return checkDataConsistency(GetInput(), GetOutput());
}

}  // namespace shkryleva_s_qsort_smerge
