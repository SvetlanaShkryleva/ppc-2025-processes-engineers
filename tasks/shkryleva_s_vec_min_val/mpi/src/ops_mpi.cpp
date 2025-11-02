#include "shkryleva_s_vec_min_val/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <climits>
#include <cstddef>
#include <vector>

#include "shkryleva_s_vec_min_val/common/include/common.hpp"

namespace shkryleva_s_vec_min_val {

ShkrylevaSVecMinValMPI::ShkrylevaSVecMinValMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool ShkrylevaSVecMinValMPI::ValidationImpl() {  // NOLINT
  return (!GetInput().empty()) && (GetOutput() == 0);
}

bool ShkrylevaSVecMinValMPI::PreProcessingImpl() {  // NOLINT
  int initialized = 0;
  MPI_Initialized(&initialized);
  if (initialized == 0) {
    MPI_Init(nullptr, nullptr);
  }

  GetOutput() = INT_MAX;
  return true;
}

bool ShkrylevaSVecMinValMPI::RunImpl() {  // NOLINT
  if (GetInput().empty()) {
    return false;
  }

  int world_rank = 0;
  int world_size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  int total_size = 0;
  const std::vector<int> *input_data_ptr = nullptr;  // NOLINT

  if (world_rank == 0) {
    input_data_ptr = &GetInput();
    total_size = static_cast<int>(input_data_ptr->size());
  }

  MPI_Bcast(&total_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (total_size == 0) {
    return false;
  }

  int base_local_size = total_size / world_size;
  int remainder = total_size % world_size;

  std::vector<int> sendcounts(world_size);     // NOLINT
  std::vector<int> displacements(world_size);  // NOLINT

  int offset = 0;
  for (int i = 0; i < world_size; ++i) {
    sendcounts[i] = base_local_size + (i < remainder ? 1 : 0);
    displacements[i] = offset;
    offset += sendcounts[i];
  }

  if (sendcounts[world_rank] == 0) {
    GetOutput() = INT_MAX;
    return true;
  }

  std::vector<int> local_data(sendcounts[world_rank]);  // NOLINT

  MPI_Scatterv((world_rank == 0) ? input_data_ptr->data() : nullptr, sendcounts.data(), displacements.data(), MPI_INT,
               local_data.data(), sendcounts[world_rank], MPI_INT, 0, MPI_COMM_WORLD);

  int local_min = INT_MAX;
  for (int value : local_data) {
    if (value < local_min) {
      local_min = value;
    }
  }

  int global_min = INT_MAX;
  MPI_Allreduce(&local_min, &global_min, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);

  GetOutput() = global_min;
  return true;
}

bool ShkrylevaSVecMinValMPI::PostProcessingImpl() {  // NOLINT
  int finalized = 0;
  MPI_Finalized(&finalized);
  if (finalized == 0) {
    // MPI_Finalize();
  }
  return GetOutput() > INT_MIN;
}

}  // namespace shkryleva_s_vec_min_val
