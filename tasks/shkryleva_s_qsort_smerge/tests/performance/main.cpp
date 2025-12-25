#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

#include "shkryleva_s_qsort_smerge/common/include/common.hpp"
#include "shkryleva_s_qsort_smerge/mpi/include/ops_mpi.hpp"
#include "shkryleva_s_qsort_smerge/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace shkryleva_s_qsort_smerge {

class ShkrylevaSQSortSMergePerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
 private:
  const size_t kArraySize_ = 1000000;
  InType input_data_;

 protected:
  void SetUp() override {
    input_data_.resize(kArraySize_);
    for (size_t i = 0; i < kArraySize_; i++) {
      input_data_[i] = static_cast<int>(kArraySize_ - i);
    }
  }

  bool CheckTestOutputData(OutType &output_data) override {
    if (output_data.size() != input_data_.size()) {
      return false;
    }

    if (!std::is_sorted(output_data.begin(), output_data.end())) {
      return false;
    }

    std::vector<int> expected = input_data_;
    std::sort(expected.begin(), expected.end());

    return output_data == expected;
  }

  InType GetTestInputData() override {
    return input_data_;
  }
};

TEST_P(ShkrylevaSQSortSMergePerfTests, TaskRun) {
  ExecuteTest(GetParam());
}

TEST_P(ShkrylevaSQSortSMergePerfTests, TaskPipeline) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, ShkrylevaSQSortSMergeMPI, ShkrylevaSQSortSMergeSEQ>(
    PPC_SETTINGS_shkryleva_s_qsort_smerge);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = ShkrylevaSQSortSMergePerfTests::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, ShkrylevaSQSortSMergePerfTests, kGtestValues, kPerfTestName);

}  // namespace shkryleva_s_qsort_smerge
