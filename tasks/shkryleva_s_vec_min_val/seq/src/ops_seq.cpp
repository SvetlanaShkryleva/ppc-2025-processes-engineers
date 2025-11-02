#include "shkryleva_s_vec_min_val/seq/include/ops_seq.hpp"

#include <algorithm>
#include <climits>
#include <vector>

#include "shkryleva_s_vec_min_val/common/include/common.hpp"

namespace shkryleva_s_vec_min_val {

ShkrylevaSVecMinValSEQ::ShkrylevaSVecMinValSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool ShkrylevaSVecMinValSEQ::ValidationImpl() {  // NOLINT
  return (!GetInput().empty()) && (GetOutput() == 0);
}

bool ShkrylevaSVecMinValSEQ::PreProcessingImpl() {  // NOLINT
  GetOutput() = INT_MAX;
  return true;
}

bool ShkrylevaSVecMinValSEQ::RunImpl() {  // NOLINT
  if (GetInput().empty()) {
    return false;
  }
  int min_val = *std::min_element(GetInput().begin(), GetInput().end());
  GetOutput() = min_val;
  return true;
}

bool ShkrylevaSVecMinValSEQ::PostProcessingImpl() {  // NOLINT
  return GetOutput() > INT_MIN;
}

}  // namespace shkryleva_s_vec_min_val
