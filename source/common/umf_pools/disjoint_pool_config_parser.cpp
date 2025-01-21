//===--- disjoint_pool_config_parser.cpp -configuration for USM memory pool-==//
//
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM
// Exceptions. See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "disjoint_pool_config_parser.hpp"

#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

#include "ur_util.hpp"

namespace usm {
constexpr auto operator""_B(unsigned long long x) -> size_t { return x; }
constexpr auto operator""_KB(unsigned long long x) -> size_t {
  return x * 1024;
}
constexpr auto operator""_MB(unsigned long long x) -> size_t {
  return x * 1024 * 1024;
}
constexpr auto operator""_GB(unsigned long long x) -> size_t {
  return x * 1024 * 1024 * 1024;
}

umf_disjoint_pool_config_t::umf_disjoint_pool_config_t()
    : SlabMinSize(0), MaxPoolableSize(0), Capacity(0),
      MinBucketSize(UMF_DISJOINT_POOL_MIN_BUCKET_DEFAULT_SIZE), PoolTrace(0),
      SharedLimits(nullptr), Name("disjoint_pool") {}

DisjointPoolAllConfigs::DisjointPoolAllConfigs(int trace) {
  for (auto &Config : Configs) {
    Config.PoolTrace = trace;
  }

  Configs[DisjointPoolMemType::Host].Name = "Host";
  Configs[DisjointPoolMemType::Device].Name = "Device";
  Configs[DisjointPoolMemType::Shared].Name = "Shared";
  Configs[DisjointPoolMemType::SharedReadOnly].Name = "SharedReadOnly";

  // Buckets for Host use a minimum of the cache line size of 64 bytes.
  // This prevents two separate allocations residing in the same cache line.
  // Buckets for Device and Shared allocations will use starting size of 512.
  // This is because memory compression on newer GPUs makes the
  // minimum granularity 512 bytes instead of 64.
  Configs[DisjointPoolMemType::Host].MinBucketSize = 64;
  Configs[DisjointPoolMemType::Device].MinBucketSize = 512;
  Configs[DisjointPoolMemType::Shared].MinBucketSize = 512;
  Configs[DisjointPoolMemType::SharedReadOnly].MinBucketSize = 512;

  // Initialize default pool settings.
  Configs[DisjointPoolMemType::Host].MaxPoolableSize = 2_MB;
  Configs[DisjointPoolMemType::Host].Capacity = 4;
  Configs[DisjointPoolMemType::Host].SlabMinSize = 64_KB;

  Configs[DisjointPoolMemType::Device].MaxPoolableSize = 4_MB;
  Configs[DisjointPoolMemType::Device].Capacity = 4;
  Configs[DisjointPoolMemType::Device].SlabMinSize = 64_KB;

  // Disable pooling of shared USM allocations.
  Configs[DisjointPoolMemType::Shared].MaxPoolableSize = 0;
  Configs[DisjointPoolMemType::Shared].Capacity = 0;
  Configs[DisjointPoolMemType::Shared].SlabMinSize = 2_MB;

  // Allow pooling of shared allocations that are only modified on host.
  Configs[DisjointPoolMemType::SharedReadOnly].MaxPoolableSize = 4_MB;
  Configs[DisjointPoolMemType::SharedReadOnly].Capacity = 4;
  Configs[DisjointPoolMemType::SharedReadOnly].SlabMinSize = 2_MB;
}

std::optional<size_t> stringToNumber(std::string_view s) {
  auto unitPos = s.find_first_of("kKmMgG");
  size_t multiplier = 1;
  if (unitPos != std::string_view::npos) {
    switch (tolower(s[unitPos])) {
    case 'k':
      multiplier = 1_KB;
      break;
    case 'm':
      multiplier = 1_MB;
      break;
    case 'g':
      multiplier = 1_GB;
      break;
    }
  }

  try {
    return std::stoull(std::string(s.substr(0, unitPos))) * multiplier;
  } catch (...) {
    return std::nullopt;
  }
}

std::pair<std::optional<size_t>, std::string_view>
maybeParseNumber(std::string_view s) {
  auto separator = ';';
  auto separatorPos = s.find(separator);
  if (separatorPos == std::string_view::npos) {
    auto number = stringToNumber(s.substr(0, separatorPos));
    if (number)
      return {number, std::string_view()};
    else
      return {std::nullopt, s};
  }

  return {stringToNumber(s.substr(0, separatorPos)),
          s.substr(separatorPos + 1)};
}

DisjointPoolMemType parseMemType(std::string_view s) {
  if (s == "host")
    return DisjointPoolMemType::Host;
  if (s == "device")
    return DisjointPoolMemType::Device;
  if (s == "shared")
    return DisjointPoolMemType::Shared;
  if (s == "read_only_shared")
    return DisjointPoolMemType::SharedReadOnly;

  throw std::invalid_argument("Unknown memory type: " + std::string(s));
}

DisjointPoolAllConfigs parseDisjointPoolConfig(const std::string &config,
                                               int trace) {
  DisjointPoolAllConfigs AllConfigs;

  std::optional<size_t> Buffers, MaxSize;
  std::string input;

  std::tie(Buffers, input) = maybeParseNumber(config);
  std::tie(MaxSize, input) = maybeParseNumber(input);

  auto setConfigValues = [](umf_disjoint_pool_config_t &config,
                            std::vector<std::string> values) {
    if (values.size() > 0)
      config.MaxPoolableSize = stringToNumber(values[0]).value();
    if (values.size() > 1)
      config.Capacity = stringToNumber(values[1]).value();
    if (values.size() > 2)
      config.SlabMinSize = stringToNumber(values[2]).value();
  };

  try {
    // try to parse the string with per-type settings

    auto perTypeSettings = parse_string_to_map(input, false);
    for (auto &[type, values] : perTypeSettings) {
      DisjointPoolMemType memType = parseMemType(type);

      auto &config = AllConfigs.Configs[memType];
      if (values.size() > 3)
        throw std::invalid_argument("Too many values for memory type " + type);

      setConfigValues(config, values);
    }
  } catch (std::invalid_argument &) {
    // if parsing per-type failed, try to parse the string with settings for all
    // types

    auto allTypeSettings = parse_string_to_vec(input);
    if (allTypeSettings.size() > 3)
      throw std::invalid_argument("Too many values for memory type settings");

    for (auto &Config : AllConfigs.Configs) {
      setConfigValues(Config, allTypeSettings);
    }
  }

  AllConfigs.EnableBuffers = Buffers.value_or(1);

  AllConfigs.limits = std::shared_ptr<umf_disjoint_pool_shared_limits_t>(
      umfDisjointPoolSharedLimitsCreate(
          MaxSize.value_or(std::numeric_limits<size_t>::max())),
      umfDisjointPoolSharedLimitsDestroy);

  for (auto &Config : AllConfigs.Configs) {
    Config.SharedLimits = AllConfigs.limits.get();
    Config.PoolTrace = trace;
  }

  if (!trace) {
    return AllConfigs;
  }

  std::cout << "USM Pool Settings (Built-in or Adjusted by Environment "
               "Variable)"
            << std::endl;

  std::cout << std::setw(15) << "Parameter" << std::setw(12) << "Host"
            << std::setw(12) << "Device" << std::setw(12) << "Shared RW"
            << std::setw(12) << "Shared RO" << std::endl;
  std::cout
      << std::setw(15) << "SlabMinSize" << std::setw(12)
      << AllConfigs.Configs[DisjointPoolMemType::Host].SlabMinSize
      << std::setw(12)
      << AllConfigs.Configs[DisjointPoolMemType::Device].SlabMinSize
      << std::setw(12)
      << AllConfigs.Configs[DisjointPoolMemType::Shared].SlabMinSize
      << std::setw(12)
      << AllConfigs.Configs[DisjointPoolMemType::SharedReadOnly].SlabMinSize
      << std::endl;
  std::cout
      << std::setw(15) << "MaxPoolableSize" << std::setw(12)
      << AllConfigs.Configs[DisjointPoolMemType::Host].MaxPoolableSize
      << std::setw(12)
      << AllConfigs.Configs[DisjointPoolMemType::Device].MaxPoolableSize
      << std::setw(12)
      << AllConfigs.Configs[DisjointPoolMemType::Shared].MaxPoolableSize
      << std::setw(12)
      << AllConfigs.Configs[DisjointPoolMemType::SharedReadOnly].MaxPoolableSize
      << std::endl;
  std::cout << std::setw(15) << "Capacity" << std::setw(12)
            << AllConfigs.Configs[DisjointPoolMemType::Host].Capacity
            << std::setw(12)
            << AllConfigs.Configs[DisjointPoolMemType::Device].Capacity
            << std::setw(12)
            << AllConfigs.Configs[DisjointPoolMemType::Shared].Capacity
            << std::setw(12)
            << AllConfigs.Configs[DisjointPoolMemType::SharedReadOnly].Capacity
            << std::endl;
  std::cout << std::setw(15) << "MaxPoolSize" << std::setw(12)
            << MaxSize.value_or(std::numeric_limits<size_t>::max())
            << std::endl;
  std::cout << std::setw(15) << "EnableBuffers" << std::setw(12)
            << AllConfigs.EnableBuffers << std::endl
            << std::endl;

  return AllConfigs;
}
} // namespace usm
