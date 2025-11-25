#include "shkryleva_s_vec_min_val/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <climits>
#include <cstdint>
#include <vector>

#include "shkryleva_s_vec_min_val/common/include/common.hpp"

namespace shkryleva_s_vec_min_val {

ShkrylevaSVecMinValMPI::ShkrylevaSVecMinValMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool ShkrylevaSVecMinValMPI::ValidationImpl() {
  int world_rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  bool is_valid = true;

  if (world_rank == 0) {
    is_valid = !GetInput().empty();
  }

  int validation_result = is_valid ? 1 : 0;
  MPI_Bcast(&validation_result, 1, MPI_INT, 0, MPI_COMM_WORLD);

  return validation_result != 0;
}

bool ShkrylevaSVecMinValMPI::PreProcessingImpl() {
  GetOutput() = INT_MAX;
  return true;
}

bool ShkrylevaSVecMinValMPI::RunImpl() {
  int world_rank = 0;
  int world_size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  uint64_t total_size = 0;
  const std::vector<int> *input_data_ptr = nullptr;

  if (world_rank == 0) {
    input_data_ptr = &GetInput();
    total_size = static_cast<uint64_t>(input_data_ptr->size());
  }

  MPI_Bcast(&total_size, 1, MPI_UINT64_T, 0, MPI_COMM_WORLD);

  if (total_size == 0) {
    return true;
  }

  uint64_t base_size = total_size / world_size;
  uint64_t extra_items = total_size % world_size;

  std::vector<uint64_t> sendcounts(world_size);
  std::vector<uint64_t> displacements(world_size);

  uint64_t offset = 0;
  for (int i = 0; i < world_size; ++i) {
    sendcounts[i] = base_size + (i < extra_items ? 1 : 0);
    displacements[i] = offset;
    offset += sendcounts[i];
  }

  std::vector<int> sendcounts_int(world_size);
  std::vector<int> displacements_int(world_size);

  for (int i = 0; i < world_size; ++i) {
    sendcounts_int[i] = static_cast<int>(sendcounts[i]);
    displacements_int[i] = static_cast<int>(displacements[i]);
  }

  std::vector<int> local_data(std::max(sendcounts_int[world_rank], 0));

  MPI_Scatterv((world_rank == 0) ? input_data_ptr->data() : nullptr, sendcounts_int.data(), displacements_int.data(),
               MPI_INT, local_data.data(), sendcounts_int[world_rank], MPI_INT, 0, MPI_COMM_WORLD);

  int local_min = INT_MAX;
  if (sendcounts_int[world_rank] > 0) {
    for (int value : local_data) {
      local_min = (value < local_min) ? value : local_min;
    }
  }

  int total_min = INT_MAX;
  MPI_Allreduce(&local_min, &total_min, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);

  GetOutput() = total_min;
  return true;
}

bool ShkrylevaSVecMinValMPI::PostProcessingImpl() {
  return GetOutput() > INT_MIN;
}

}  // namespace shkryleva_s_vec_min_val
