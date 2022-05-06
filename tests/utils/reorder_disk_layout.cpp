// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <atomic>
#include <cstring>
#include <iomanip>
#include <omp.h>
#include <pq_flash_index.h>
#include <set>
#include <string.h>
#include <time.h>

#include "aux_utils.h"
#include "index.h"
#include "math_utils.h"
#include "memory_mapper.h"
#include "partition_and_pq.h"
#include "timer.h"
#include "utils.h"
#include <numeric>

#ifndef _WINDOWS
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "linux_aligned_file_reader.h"
#else
#ifdef USE_BING_INFRA
#include "bing_aligned_file_reader.h"
#else
#include "windows_aligned_file_reader.h"
#endif
#endif

#define WARMUP false

void print_stats(std::string category, std::vector<float> percentiles,
                 std::vector<float> results) {
  diskann::cout << std::setw(20) << category << ": " << std::flush;
  for (uint32_t s = 0; s < percentiles.size(); s++) {
    diskann::cout << std::setw(8) << percentiles[s] << "%";
  }
  diskann::cout << std::endl;
  diskann::cout << std::setw(22) << " " << std::flush;
  for (uint32_t s = 0; s < percentiles.size(); s++) {
    diskann::cout << std::setw(9) << results[s];
  }
  diskann::cout << std::endl;
}

template<typename T>
int search_disk_index(int argc, char** argv) {
  // load query bin

  _u32            ctr = 2;

  std::string index_prefix_path(argv[ctr++]);
  std::string pq_prefix = index_prefix_path + "_pq";
  std::string disk_index_file = index_prefix_path + "_disk.index";
  std::string out_file(argv[ctr++]);

  diskann::Metric metric = diskann::Metric::L2;
 std::shared_ptr<AlignedFileReader> reader = nullptr;
 _u32 num_threads = 1;
#ifdef _WINDOWS
#ifndef USE_BING_INFRA
  reader.reset(new WindowsAlignedFileReader());
#else
  reader.reset(new diskann::BingAlignedFileReader());
#endif
#else
  reader.reset(new LinuxAlignedFileReader());
#endif

  std::unique_ptr<diskann::PQFlashIndex<T>> _pFlashIndex(
      new diskann::PQFlashIndex<T>(reader, metric));

  int res = _pFlashIndex->load(num_threads, pq_prefix.c_str(),
                               disk_index_file.c_str());

  if (res != 0) {
    return res;
  }

  std::vector<_u32> output_order;
//  std::iota(output_order.begin(), output_order.end(), 0);
  _pFlashIndex->generate_new_disk_ordering(output_order);
  _pFlashIndex->reorder_disk_layout(output_order, out_file);
  return 0;
}

int main(int argc, char** argv) {
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0]
              << "   index_type<float/int8/uint8>     "
                 "index_prefix_path  output_index_prefix "
              << std::endl;
    exit(-1);
  }
  try {
    if (std::string(argv[1]) == std::string("float"))
      return search_disk_index<float>(argc, argv);
    else if (std::string(argv[1]) == std::string("int8"))
      return search_disk_index<int8_t>(argc, argv);
    else if (std::string(argv[1]) == std::string("uint8"))
      return search_disk_index<uint8_t>(argc, argv);
    else {
      std::cerr << "Unsupported index type. Use float or int8 or uint8"
                << std::endl;
      return -1;
    }
  } catch (const std::exception& e) {
    std::cout << std::string(e.what()) << std::endl;
    diskann::cerr << "Index search failed." << std::endl;
    return -1;
  }
}