#pragma once

#include <cmath>
#include <memory>
#include <span>
#include <vector>

#include "shkryleva_s_qsort_smerge/common/include/common.hpp"
#include "task/include/task.hpp"

namespace shkryleva_s_qsort_smerge {

class ShkrylevaSQSortSMergeSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit ShkrylevaSQSortSMergeSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
  std::vector<int> input_;
  std::vector<int> output_;

  static std::vector<int> Merge(const std::vector<int> &left, const std::vector<int> &right);
  std::vector<int> QuickSortWithMerge(const std::span<int> &arr);
};

}  // namespace shkryleva_s_qsort_smerge
