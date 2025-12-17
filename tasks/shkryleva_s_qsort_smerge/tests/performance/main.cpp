#include <gtest/gtest.h>

#include <algorithm>
#include <numeric>
#include <random>
#include <vector>

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
    input_data_.resize(kCount_);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 10000);

    for (int i = 0; i < kCount_; ++i) {
      input_data_[i] = dist(gen);
    }
  }

  bool CheckTestOutputData(OutType &output_data) override {
    if (input_data_.size() != output_data.size()) {
      return false;
    }

    if (!std::is_sorted(output_data.begin(), output_data.end())) {
      return false;
    }

    std::vector<int> input_copy = input_data_;
    std::vector<int> output_copy = output_data;

    std::sort(input_copy.begin(), input_copy.end());
    std::sort(output_copy.begin(), output_copy.end());

    return input_copy == output_copy;
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
