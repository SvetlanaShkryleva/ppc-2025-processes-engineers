#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <string>
#include <tuple>

#include "shkryleva_s_seidel_method/common/include/common.hpp"
#include "shkryleva_s_seidel_method/mpi/include/ops_mpi.hpp"
#include "shkryleva_s_seidel_method/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace shkryleva_s_seidel_method {

class ShkrylevaRunFuncTestsSeidelMethod : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static auto PrintTestParam(const TestType &test_param) -> std::string {
    return std::to_string(std::get<0>(test_param)) + "_" + std::get<1>(test_param);
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    int matrix_size = std::get<0>(params);
    test_name_ = std::get<1>(params);

    input_data_ = matrix_size;

    expected_output_ = 1;
  }

  auto CheckTestOutputData(OutType &output_data) -> bool final {
    return (output_data > 0);
  }

  auto GetTestInputData() -> InType final {
    return input_data_;
  }

 private:
  InType input_data_;
  OutType expected_output_;
  std::string test_name_;
};

namespace {

TEST_P(ShkrylevaRunFuncTestsSeidelMethod, GaussSeidelConvergenceTest) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 6> kTestParam = {std::make_tuple(3, "small_matrix"),    std::make_tuple(5, "medium_matrix"),
                                            std::make_tuple(10, "large_matrix"),   std::make_tuple(15, "xlarge_matrix"),
                                            std::make_tuple(20, "xxlarge_matrix"), std::make_tuple(25, "huge_matrix")};

const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<ShkrylevaSSeidelMethodMPI, InType>(kTestParam, PPC_SETTINGS_shkryleva_s_seidel_method),
    ppc::util::AddFuncTask<ShkrylevaSSeidelMethodSEQ, InType>(kTestParam, PPC_SETTINGS_shkryleva_s_seidel_method));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = ShkrylevaRunFuncTestsSeidelMethod::PrintFuncTestName<ShkrylevaRunFuncTestsSeidelMethod>;

INSTANTIATE_TEST_SUITE_P(GaussSeidelTests, ShkrylevaRunFuncTestsSeidelMethod, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace shkryleva_s_seidel_method
