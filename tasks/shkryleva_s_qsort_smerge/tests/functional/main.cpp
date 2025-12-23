#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <ranges>
#include <string>
#include <tuple>
#include <vector>

#include "shkryleva_s_qsort_smerge/common/include/common.hpp"
#include "shkryleva_s_qsort_smerge/mpi/include/ops_mpi.hpp"
#include "shkryleva_s_qsort_smerge/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace shkryleva_s_qsort_smerge {

class ShkrylevaSQSortSMergeFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::get<1>(test_param);
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    input_data_ = std::get<0>(params);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    if (input_data_.size() != output_data.size()) {
      return false;
    }

    for (size_t i = 1; i < output_data.size(); ++i) {
      if (output_data[i - 1] > output_data[i]) {
        return false;
      }
    }

    std::vector<int> input_copy = input_data_;
    std::vector<int> output_copy = output_data;
    std::ranges::sort(input_copy);
    std::ranges::sort(output_copy);

    return input_copy == output_copy;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
};

namespace {

TEST_P(ShkrylevaSQSortSMergeFuncTests, QSortSMergeTest) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 9> kTestParam = {
    std::make_tuple(std::vector<int>{5, 3, 8, 6, 2, 7}, "test_sort_array"),
    std::make_tuple(std::vector<int>{42}, "test_single_element"),
    std::make_tuple(std::vector<int>{7, 7, 7, 7, 7, 7, 7, 7}, "test_all_equal_elements"),
    std::make_tuple(std::vector<int>{-3, 2, -1, 0, 1}, "test_negative_and_positive"),

    std::make_tuple(std::vector<int>{}, "test_empty_array"),
    std::make_tuple(std::vector<int>{1, 2, 3, 4, 5}, "test_already_sorted"),
    std::make_tuple(std::vector<int>{5, 4, 3, 2, 1}, "test_reverse_sorted"),

    std::make_tuple(std::vector<int>{9, 5, 2, 8, 1, 7, 3, 6, 4, 0}, "test_medium_size"),
    std::make_tuple(std::vector<int>{100, -50, 75, -25, 0, 33, -66, 99, -99, 50, -75, 25, -33, 66, -100},
                    "test_mixed_values")};

const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<ShkrylevaSQSortSMergeMPI, InType>(kTestParam, PPC_SETTINGS_shkryleva_s_qsort_smerge),
    ppc::util::AddFuncTask<ShkrylevaSQSortSMergeSEQ, InType>(kTestParam, PPC_SETTINGS_shkryleva_s_qsort_smerge));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = ShkrylevaSQSortSMergeFuncTests::PrintFuncTestName<ShkrylevaSQSortSMergeFuncTests>;

INSTANTIATE_TEST_SUITE_P(SQSortSMergeTests, ShkrylevaSQSortSMergeFuncTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace shkryleva_s_qsort_smerge
