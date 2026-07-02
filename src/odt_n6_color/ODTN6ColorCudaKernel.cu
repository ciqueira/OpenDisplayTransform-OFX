// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 Magno Ciqueira.
// Portions of the image math are derived from open-display-transform by Jed Smith.

#include "ODTN6ColorMath.h"
#include "ODTN6ColorParams.h"

#include <cuda_runtime.h>

__global__ void ODTN6ColorCudaKernel(int width, int height, int rowBytes,
                                     const float *input, float *output,
                                     ODTN6ColorParams params) {
  const int x = blockIdx.x * blockDim.x + threadIdx.x;
  const int y = blockIdx.y * blockDim.y + threadIdx.y;
  if (x >= width || y >= height) {
    return;
  }

  const int floatsPerRow = rowBytes / static_cast<int>(sizeof(float));
  const int idx = y * floatsPerRow + x * 4;

  const odtn6color::float3 src =
      odtn6color::make_float3(input[idx + 0], input[idx + 1], input[idx + 2]);
  const odtn6color::float3 out = odtn6color::applyN6Color(src, params);

  output[idx + 0] = out.x;
  output[idx + 1] = out.y;
  output[idx + 2] = out.z;
  output[idx + 3] = input[idx + 3];
}

extern "C" void RunODTN6ColorCudaKernel(void *stream, int width, int height,
                                        int rowBytes, const float *input,
                                        float *output,
                                        ODTN6ColorParams params) {
  const dim3 block(16, 16);
  const dim3 grid((width + block.x - 1) / block.x,
                  (height + block.y - 1) / block.y);
  cudaStream_t cudaStream = static_cast<cudaStream_t>(stream);
  ODTN6ColorCudaKernel<<<grid, block, 0, cudaStream>>>(
      width, height, rowBytes, input, output, params);
}
