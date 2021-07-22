// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <string>
#include <iostream>
#include <fstream>
#include <cassert>


template<typename T>
inline void load_bin(const char *filename, T *&data, size_t &npts,
                     size_t &ndims) {
  std::ifstream reader(filename, std::ios::binary);
  std::cout << "Reading bin file " << filename << " ...\n";
  int npts_i32, ndims_i32;
  reader.read((char *) &npts_i32, sizeof(int));
  reader.read((char *) &ndims_i32, sizeof(int));

  npts = (size_t) npts_i32;
  ndims = (size_t) ndims_i32;

  data = new T[npts * ndims];
  reader.read((char *) data, sizeof(T) * npts * ndims);
  std::cout << "Finished reading the bin file." << std::endl;
  reader.close();
}

template<typename T_out, typename T_in>
void save_groundtruth_as_csv(const std::string filename, unsigned *ids,
                             T_in *base_data, T_in *query_data, size_t nqueries,
                             size_t ngt, size_t ndims) {
  std::ofstream writer(filename, std::ios::out);

  for (size_t q = 0; q < nqueries; ++q) {
    writer << "qv:\t";
    for (size_t d = 0; d < ndims; ++d) {
      writer << (T_out) query_data[q * ndims + d];
      if (d < ndims - 1)
        writer << ",";
      else
        writer << "\n";
    }
    for (size_t i = 0; i < ngt; ++i) {
      writer << "v:\t";
      auto vec = ids[q * ngt + i];
      for (size_t d = 0; d < ndims; ++d) {
        writer << (T_out) base_data[vec * ndims + d];
        if (d < ndims - 1)
          writer << ",";
        else
          writer << "\n";
      }
    }
  }
  writer.close();
  std::cout << "Finished writing truthset" << std::endl;
}

int main(int argc, char **argv) {
  if (argc != 6) {
    std::cerr << "Usage:  write_gt_vec <int8/uint8/float> <GT_file> "
                 "<base_data_file> <quer_data_file> <output_file>"
              << std::endl;
    exit(-1);
  }
  unsigned *gt_ids;
  size_t    npts, nqueries, ndims, K;
  load_bin<unsigned>(argv[2], gt_ids, nqueries, K);

  if (std::string(argv[1]) == std::string("int8")) {
    int8_t *base_data;
    int8_t *query_data;
    load_bin<int8_t>(argv[3], base_data, npts, ndims);
    load_bin<int8_t>(argv[4], query_data, nqueries, ndims);
    save_groundtruth_as_csv<int, int8_t>(std::string(argv[5]), gt_ids,
                                         base_data, query_data, nqueries, K,
                                         ndims);

    delete[] base_data;
    delete[] query_data;
  } else if (std::string(argv[1]) == std::string("uint8")) {
    uint8_t *base_data;
    uint8_t *query_data;
    load_bin<uint8_t>(argv[3], base_data, npts, ndims);
    load_bin<uint8_t>(argv[4], query_data, nqueries, ndims);
    save_groundtruth_as_csv<int, uint8_t>(std::string(argv[5]), gt_ids,
                                          base_data, query_data, nqueries, K,
                                          ndims);

    delete[] base_data;
    delete[] query_data;
  } else if (std::string(argv[1]) == std::string("float")) {
    float *base_data;
    float *query_data;
    load_bin<float>(argv[3], base_data, npts, ndims);
    load_bin<float>(argv[4], query_data, nqueries, ndims);
    save_groundtruth_as_csv<float, float>(std::string(argv[5]), gt_ids,
                                          base_data, query_data, nqueries, K,
                                          ndims);

    delete[] base_data;
    delete[] query_data;
  }

  delete[] gt_ids;
}
