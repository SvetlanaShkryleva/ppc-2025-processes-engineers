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
  static std::vector<int> Merge(const std::vector<int> &left, const std::vector<int> &right);
  static std::vector<int> QuickSortWithMerge(const std::vector<int> &arr);
};

}  // namespace shkryleva_s_qsort_smerge
