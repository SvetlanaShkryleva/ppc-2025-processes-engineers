#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace shkryleva_s_qsort_smerge {

using InType = std::vector<int>;
using OutType = std::vector<int>;
using TestType = std::tuple<InType, std::string> using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace shkryleva_s_qsort_smerge
