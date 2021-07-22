// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <iostream>
#include "utils.h"


void block_convert(std::ofstream& writer, int8_t* write_buf,
                   std::ifstream& reader, float* read_buf, _u64 npts,
                   _u64 ndims, float bias, float scale, bool normalize) {
  reader.read((char*) read_buf, npts * ndims * sizeof(float));

  for (_u64 i = 0; i < npts; i++) {
    
    float norm = 1.0;
    if (normalize) {
      float sq_norm = 0.0;
      for (_u64 d = 0; d < ndims; d++) {
        sq_norm += read_buf[d + i * ndims] * read_buf[d + i * ndims];
      }
      norm = std::sqrt(sq_norm);
    }
    for (_u64 d = 0; d < ndims; d++) {
      write_buf[d + i * ndims] =
          (int8_t)((read_buf[d + i * ndims] / norm - bias) * (254.0/scale));
    }
  }
  writer.write((char*) write_buf, npts * ndims);
}

int main(int argc, char** argv) {
  if (argc != 6) {
    std::cout << "Usage: " << argv[0] << "  input_bin  output_tsv  bias  scale normalize<0/1>"
              << std::endl;
    exit(-1);
  }

  std::ifstream reader(argv[1], std::ios::binary);
  _u32          npts_u32;
  _u32          ndims_u32;
  reader.read((char*) &npts_u32, sizeof(_s32));
  reader.read((char*) &ndims_u32, sizeof(_s32));
  size_t npts = npts_u32;
  size_t ndims = ndims_u32;
  std::cout << "Dataset: #pts = " << npts << ", # dims = " << ndims
            << std::endl;

  _u64 blk_size = 131072;
  _u64 nblks = ROUND_UP(npts, blk_size) / blk_size;

  std::ofstream writer(argv[2], std::ios::binary);
  auto          read_buf = new float[blk_size * ndims];
  auto          write_buf = new int8_t[blk_size * ndims];
  float         bias = atof(argv[3]);
  float         scale = atof(argv[4]);
  bool          normalize = atoi(argv[5]);

  writer.write((char*) (&npts_u32), sizeof(_u32));
  writer.write((char*) (&ndims_u32), sizeof(_u32));

  for (_u64 i = 0; i < nblks; i++) {
    _u64 cblk_size = std::min(npts - i * blk_size, blk_size);
    block_convert(writer, write_buf, reader, read_buf, cblk_size, ndims, bias,
                  scale, normalize);
    std::cout << "Block #" << i << " written" << std::endl;
  }

  delete[] read_buf;
  delete[] write_buf;

  writer.close();
  reader.close();
}
