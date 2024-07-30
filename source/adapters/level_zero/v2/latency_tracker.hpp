//===--------- ur_latency_tracker.cpp - common ---------------------------===//
//
// Copyright (C) 2024 Intel Corporation
//
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM
// Exceptions. See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#pragma once

#include <chrono>
#include <cstdint>
#include <limits>
#include <memory>

#include "logger/ur_logger.hpp"
#include <hdr/hdr_histogram.h>

namespace v2 {

static inline bool trackLatency = []() {
  return std::getenv("UR_ENABLE_LATENCY_TRACKING") != nullptr;
}();

static constexpr double percentiles[7] = {50.0, 90.0, 99.0, 99.9, 99.99, 99.999, 99.9999};

struct latencyValues {
  const char* name;
  int64_t min;
  int64_t max;
  double mean;
  double stddev;
  int64_t percentileValues[7];
};

class latency_printer {
public:
  latency_printer() = default;

  inline void publishLatency(latencyValues value) {
    values.push_back(value);
  }

  inline ~latency_printer() {
    print();
  }

  inline void print() {
    printHeader();
    for(auto &value : values) {
      logger::info(
          "{},{},{},{},{},{},{},{},{},{},{},{},ns",
          value.name, value.min, value.max, value.mean, value.stddev,
          value.percentileValues[0], value.percentileValues[1],
          value.percentileValues[2], value.percentileValues[3],
          value.percentileValues[4], value.percentileValues[5],
          value.percentileValues[6]);
    }
  }

private:
  inline void printHeader() {
    logger::info(
        "name,min,max,mean,stdev,{},{},{},{},{},{},{},unit",
        percentiles[0], percentiles[1], percentiles[2], percentiles[3],
        percentiles[4], percentiles[5], percentiles[6]);
  }

  std::vector<latencyValues> values;
};

// TODO: make this a member somewhere?
static inline latency_printer& globalPrinter() {
  static latency_printer printer;
  return printer;
}

class latency_histogram {
public:
  inline latency_histogram(const char *name, latency_printer& printer = globalPrinter(), int64_t lowestDiscernibleValue = 1,
                    int64_t highestTrackableValue = 1000'000'000,
                    int significantFigures = 3)
      : name(name), printer(printer) {
    if (trackLatency) {
      auto ret = hdr_init(lowestDiscernibleValue, highestTrackableValue,
                          significantFigures, &histogram);
      if (ret != 0) {
        logger::error("Failed to initialize latency histogram");
        histogram = nullptr;
      }
    }
  }

  latency_histogram(const latency_histogram &) = delete;
  latency_histogram(latency_histogram &&) = delete;

  inline ~latency_histogram() {
    if (!trackLatency || !histogram) {
      return;
    }

    if (hdr_min(histogram) == std::numeric_limits<int64_t>::max()) {
      logger::info("[{}] latency: no data", name);
      hdr_close(histogram);
      return;
    }

    latencyValues values;

    values.name = name;
    values.max = hdr_max(histogram);
    values.min = hdr_min(histogram);
    values.mean = hdr_mean(histogram);
    values.stddev = hdr_stddev(histogram);

    auto ret = hdr_value_at_percentiles(histogram, percentiles, values.percentileValues, 7);
    if (ret != 0) {
      hdr_close(histogram);
      logger::error("Failed to get percentiles from latency histogram");
      return;
    }

    printer.publishLatency(values);

    hdr_close(histogram);
  }

  inline void trackValue(int64_t value) { hdr_record_value(histogram, value); }

private:
  const char *name;
  struct hdr_histogram *histogram;
  latency_printer& printer;
};

class latency_tracker {
public:
  explicit latency_tracker(latency_histogram &stats)
      : stats(trackLatency ? &stats : nullptr), begin() {
    if (trackLatency) {
      begin = std::chrono::steady_clock::now();
    }
  }
  latency_tracker() {}
  ~latency_tracker() {
    if (stats) {
      auto tp = std::chrono::steady_clock::now();
      auto diffNanos =
          std::chrono::duration_cast<std::chrono::nanoseconds>(tp - begin)
              .count();
      stats->trackValue(static_cast<int64_t>(diffNanos));
    }
  }

  latency_tracker(const latency_tracker &) = delete;
  latency_tracker &operator=(const latency_tracker &) = delete;

  latency_tracker(latency_tracker &&rhs) noexcept
      : stats(rhs.stats), begin(rhs.begin) {
    rhs.stats = nullptr;
  }

  latency_tracker &operator=(latency_tracker &&rhs) noexcept {
    if (this != &rhs) {
      this->~latency_tracker();
      new (this) latency_tracker(std::move(rhs));
    }
    return *this;
  }

private:
  latency_histogram *stats{nullptr};
  std::chrono::time_point<std::chrono::steady_clock> begin;
};

} // namespace v2
