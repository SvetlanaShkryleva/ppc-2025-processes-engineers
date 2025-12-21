#include <gtest/gtest.h>

#include "shkryleva_s_qsort_smerge/common/include/common.hpp"
#include "shkryleva_s_qsort_smerge/mpi/include/ops_mpi.hpp"
#include "shkryleva_s_qsort_smerge/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace shkryleva_s_qsort_smerge {

class ShkrylevaSQSortSMergePerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
 private:
  const int kCount_ = 1000;
  InType input_data_{};

 protected:
  void SetUp() override {
    input_data_ = kCount_;
  }

  bool CheckTestOutputData(OutType &output_data) override {
    return input_data_ == output_data;
  }

  InType GetTestInputData() override {
    return input_data_;
  }
};

TEST_P(ShkrylevaSQSortSMergePerfTests, QSortSMergeTest) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, ShkrylevaSQSortSMergeMPI, ShkrylevaSQSortSMergeSEQ>(
    PPC_SETTINGS_shkryleva_s_qsort_smerge);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = ShkrylevaSQSortSMergePerfTests::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, ShkrylevaSQSortSMergePerfTests, kGtestValues, kPerfTestName);

}  // namespace shkryleva_s_qsort_smerge
