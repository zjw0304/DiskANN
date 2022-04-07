// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "utils.h"
#include "math_utils.h"
#include "partition_and_pq.h"
#include <random>

#define KMEANS_ITERS_FOR_PQ 15


int main(int argc, char** argv) {
  if (argc != 3) {
    std::cout
        << "Usage: \n"
        << argv[0]
        << "  n d "
        << std::endl;
        return -1;
  }
    const size_t      numr = (size_t) atoi(argv[1]);
    const size_t      numc = (size_t) atoi(argv[2]);

    double* mat = new double[numr*numc];

      std::random_device rd{};
    std::mt19937 gen{rd()};
        std::normal_distribution<> d;

        for (_u32 i=0; i < numr; i++) {
          for (_u32 j = 0; j < numc; j++) {
            mat[i*numc+j] = d(gen);
          }
        }
    math_utils:: singular_value_decomposition(numr, numc, numc, mat);
    delete[] mat;
}
