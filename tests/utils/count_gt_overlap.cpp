// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <string>
#include <iostream>
#include <fstream>
#include <cassert>
#include <set>

inline void load_gt_bin(const char *filename, unsigned *&ids,
  float *& dists, size_t &npts,
                     size_t &ndims) {
  std::ifstream reader(filename, std::ios::binary);
  std::cout << "Reading bin file " << filename << " ...\n";
  int npts_i32, ndims_i32;
  reader.read((char *) &npts_i32, sizeof(int));
  reader.read((char *) &ndims_i32, sizeof(int));

  npts = (size_t) npts_i32;
  ndims = (size_t) ndims_i32;

  ids = new unsigned[npts * ndims];
  reader.read((char *) ids, sizeof(unsigned) * npts * ndims);
  dists = new float[npts * ndims];
  reader.read((char *) dists, sizeof(float) * npts * ndims);

  std::cout << "Finished reading the bin file." << std::endl;
  reader.close();
}


float overlap_at(size_t nq, size_t K, const unsigned *const ids1,
                 const unsigned *const ids2, const float *const dist1,
                 const float *const dist2, size_t at, bool match_on_distance) {
  size_t overlap = 0;
  std::set<unsigned> gt_set;

  if (at > K) {
    std::cerr << "Error: can not compute overlap at " << at
              << " which is greater than K=" << K << std::endl;
  }

  for (size_t i = 0; i < nq; ++i) {
    gt_set.clear();
    for (size_t k = 0; k < at; ++k)
      gt_set.insert(ids1[k + K * i]);
    for (size_t k = 0; k < at; ++k)
      if (gt_set.find(ids2[k + K * i]) != gt_set.end())
        overlap++;
      else if (match_on_distance && dist1[k + K * i] == dist2[k + K * i])
        overlap++;
  }
  return ((float) overlap / (float) nq) / ((float) at);
}

int main(int argc, char **argv) {
  if (argc < 4) {
    std::cerr << "Usage:  count_gt_overlap <GT_file1> <GT_file2> "
                 "match_on_distance_tie<0/1> <overlap@> <overlap@>.. "
              << std::endl;
    exit(-1);
  }
  unsigned *ids1, *ids2;
  float *   dist1, *dist2;
  size_t    nq1, K1, nq2, K2;
  load_gt_bin(argv[1], ids1, dist1, nq1, K1);
  load_gt_bin(argv[2], ids2, dist2, nq2, K2);
  bool match_on_distance = atoi(argv[3]);

  if (nq1 != nq2 || K1 != K2) {
    std::cerr << "Error: GT files of unequal sizes" << std::endl;
    exit(-1);
  }

  for (int i = 4; i < argc; ++i)
    std::cout << "Overlap@" << argv[i] << "  "
              << overlap_at(nq1, K1, ids1, ids2, dist1, dist2, atoi(argv[i]),
                            match_on_distance)
              << std::endl;
  std::cout << "Overlap@K  "
            << overlap_at(nq1, K1, ids1, ids2, dist1, dist2, K1,
                          match_on_distance)
            << std::endl;

  delete[] ids1;
  delete[] ids2;
  delete[] dist1;
  delete[] dist2;
}
