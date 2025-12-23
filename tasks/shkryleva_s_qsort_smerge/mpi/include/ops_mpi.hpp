#pragma once

#include <vector>

#include "shkryleva_s_qsort_smerge/common/include/common.hpp"
#include "task/include/task.hpp"

namespace shkryleva_s_qsort_smerge {

class ShkrylevaSQSortSMergeMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit ShkrylevaSQSortSMergeMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

  static void ComputeDistribution(int n, int size, std::vector<int> &counts, std::vector<int> &displs);

  static std::vector<int> MergeTwoSortedVectors(const std::vector<int> &a, const std::vector<int> &b);

  static bool CheckPartsSorted(const std::vector<int> &data, const std::vector<int> &counts,
                               const std::vector<int> &displs, int size);

  static void MergeSortedParts(std::vector<int> &data, const std::vector<int> &counts, const std::vector<int> &displs,
                               int size);
};

}  // namespace shkryleva_s_qsort_smerge
