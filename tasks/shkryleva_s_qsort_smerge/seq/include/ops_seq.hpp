#pragma once

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
};

}  // namespace shkryleva_s_qsort_smerge
