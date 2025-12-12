#pragma once

#include <vector>

#include "shkryleva_s_seidel_method/common/include/common.hpp"
#include "task/include/task.hpp"

namespace shkryleva_s_seidel_method {

class ShkrylevaSSeidelMethodSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit ShkrylevaSSeidelMethodSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

  static void GenerateRandomMatrix(size_t size, std::vector<std::vector<double>> &matrix, std::vector<double> &vector);
  [[nodiscard]] static bool Converge(const std::vector<double> &x_new, const std::vector<std::vector<double>> &a,
                                     const std::vector<double> &b, double epsilon);
};

}  // namespace shkryleva_s_seidel_method
