#include "shkryleva_s_qsort_smerge/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cstddef>
#include <vector>

#include "shkryleva_s_qsort_smerge/common/include/common.hpp"

namespace shkryleva_s_qsort_smerge {

const int MAX_DATA_SIZE = 1000000;
const int MIN_VALUE = -1000000;
const int MAX_VALUE = 1000000;

ShkrylevaSQSortSMergeMPI::ShkrylevaSQSortSMergeMPI(const InType &inputVector) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = inputVector;
  GetOutput() = std::vector<int>();
}

bool ShkrylevaSQSortSMergeMPI::ValidationImpl() {
  int processRank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &processRank);

  if (!GetOutput().empty()) {
    return false;
  }

  if (processRank == 0) {
    if (GetInput().size() > MAX_DATA_SIZE) {
      return false;
    }
    for (const auto &element : GetInput()) {
      if (element < MIN_VALUE || element > MAX_VALUE) {
        return false;
      }
    }
  }
  return true;
}

bool ShkrylevaSQSortSMergeMPI::PreProcessingImpl() {
  int processRank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &processRank);

  GetOutput() = GetInput();

  if (processRank == 0) {
    return verifyDataConsistency();
  }
  return true;
}

namespace {

void performParallelSort(std::vector<int> &dataArray, int startIndex, int endIndex) {
  if (startIndex >= endIndex) {
    return;
  }

  int pivotValue = dataArray[(startIndex + endIndex) / 2];
  int leftIndex = startIndex;
  int rightIndex = endIndex;

  while (leftIndex <= rightIndex) {
    while (dataArray[leftIndex] < pivotValue) {
      ++leftIndex;
    }
    while (dataArray[rightIndex] > pivotValue) {
      --rightIndex;
    }
    if (leftIndex <= rightIndex) {
      std::swap(dataArray[leftIndex], dataArray[rightIndex]);
      ++leftIndex;
      --rightIndex;
    }
  }

  performParallelSort(dataArray, startIndex, rightIndex);
  performParallelSort(dataArray, leftIndex, endIndex);
}

std::vector<int> combineArrays(const std::vector<int> &firstArray, const std::vector<int> &secondArray) {
  std::vector<int> combinedResult;
  combinedResult.reserve(firstArray.size() + secondArray.size());

  size_t idxFirst = 0;
  size_t idxSecond = 0;

  while (idxFirst < firstArray.size() && idxSecond < secondArray.size()) {
    if (firstArray[idxFirst] <= secondArray[idxSecond]) {
      combinedResult.push_back(firstArray[idxFirst]);
      ++idxFirst;
    } else {
      combinedResult.push_back(secondArray[idxSecond]);
      ++idxSecond;
    }
  }

  while (idxFirst < firstArray.size()) {
    combinedResult.push_back(firstArray[idxFirst]);
    ++idxFirst;
  }

  while (idxSecond < secondArray.size()) {
    combinedResult.push_back(secondArray[idxSecond]);
    ++idxSecond;
  }

  return combinedResult;
}

void distributeDataAcrossProcesses(int dataSize, int processCount, std::vector<int> &sendCounts,
                                   std::vector<int> &displacements) {
  int baseChunkSize = dataSize / processCount;
  int remainder = dataSize % processCount;

  for (int proc = 0; proc < processCount; ++proc) {
    sendCounts[proc] = baseChunkSize + (proc < remainder ? 1 : 0);
    displacements[proc] = (proc == 0) ? 0 : displacements[proc - 1] + sendCounts[proc - 1];
  }
}

void gatherAndMergeResults(std::vector<int> &globalResult, const std::vector<int> &localData, int processRank,
                           int processCount, const std::vector<int> &sendCounts) {
  if (processRank == 0) {
    globalResult = localData;

    for (int sourceProc = 1; sourceProc < processCount; ++sourceProc) {
      std::vector<int> receivedData(sendCounts[sourceProc]);
      MPI_Recv(receivedData.data(), sendCounts[sourceProc], MPI_INT, sourceProc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      globalResult = combineArrays(globalResult, receivedData);
    }

    for (int destProc = 1; destProc < processCount; ++destProc) {
      MPI_Send(globalResult.data(), static_cast<int>(globalResult.size()), MPI_INT, destProc, 1, MPI_COMM_WORLD);
    }
  } else {
    MPI_Send(localData.data(), static_cast<int>(localData.size()), MPI_INT, 0, 0, MPI_COMM_WORLD);

    int receivedSize = 0;
    MPI_Status status;
    MPI_Probe(0, 1, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_INT, &receivedSize);

    globalResult.resize(receivedSize);
    MPI_Recv(globalResult.data(), receivedSize, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
}

}  // namespace

bool ShkrylevaSQSortSMergeMPI::verifyDataConsistency() {
  if (GetOutput().size() != GetInput().size()) {
    return false;
  }

  if (!GetOutput().empty()) {
    for (size_t position = 0; position < GetOutput().size(); ++position) {
      if (GetOutput()[position] != GetInput()[position]) {
        return false;
      }
    }
  }
  return true;
}

bool ShkrylevaSQSortSMergeMPI::RunImpl() {
  int processRank = 0;
  int processCount = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &processRank);
  MPI_Comm_size(MPI_COMM_WORLD, &processCount);

  int totalDataSize = 0;
  if (processRank == 0) {
    totalDataSize = static_cast<int>(GetOutput().size());
  }

  MPI_Bcast(&totalDataSize, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (totalDataSize == 0) {
    MPI_Barrier(MPI_COMM_WORLD);
    return true;
  }

  std::vector<int> chunkSizes(processCount);
  std::vector<int> displacementOffsets(processCount);

  distributeDataAcrossProcesses(totalDataSize, processCount, chunkSizes, displacementOffsets);

  std::vector<int> localChunk(chunkSizes[processRank]);

  // Важно: если процессу не досталось данных, localChunk будет пустым
  if (processRank == 0) {
    MPI_Scatterv(GetOutput().data(), chunkSizes.data(), displacementOffsets.data(), MPI_INT, localChunk.data(),
                 chunkSizes[processRank], MPI_INT, 0, MPI_COMM_WORLD);
  } else {
    MPI_Scatterv(nullptr, chunkSizes.data(), displacementOffsets.data(), MPI_INT, localChunk.data(),
                 chunkSizes[processRank], MPI_INT, 0, MPI_COMM_WORLD);
  }

  if (!localChunk.empty()) {
    performParallelSort(localChunk, 0, static_cast<int>(localChunk.size()) - 1);
  }

  gatherAndMergeResults(GetOutput(), localChunk, processRank, processCount, chunkSizes);

  MPI_Barrier(MPI_COMM_WORLD);
  return true;
}

bool ShkrylevaSQSortSMergeMPI::validateSortedData() {
  if (GetOutput().empty()) {
    return GetInput().empty();
  }

  if (!std::is_sorted(GetOutput().begin(), GetOutput().end())) {
    return false;
  }

  if (GetOutput().size() != GetInput().size()) {
    return false;
  }

  int inputSum = 0;
  int outputSum = 0;

  for (const auto &element : GetInput()) {
    inputSum += element;
  }

  for (const auto &element : GetOutput()) {
    outputSum += element;
  }

  return inputSum == outputSum;
}

bool ShkrylevaSQSortSMergeMPI::PostProcessingImpl() {
  int processRank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &processRank);

  if (processRank == 0) {
    return validateSortedData();
  }
  return true;
}

}  // namespace shkryleva_s_qsort_smerge
