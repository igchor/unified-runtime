/*
 *
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef UR_UMA_TRACKER_HPP
#define UR_UMA_TRACKER_HPP 1

#include <uma/base.h>

#include <map>
#include <mutex>
#include <shared_mutex>

namespace uma {

struct memory_tracker {
    memory_tracker(void *) {}

    enum uma_result_t add(void *pool, const void *ptr, size_t size) {
        std::unique_lock<std::shared_mutex> lock(mtx);

        auto ret = map.try_emplace(reinterpret_cast<uintptr_t>(ptr), size, pool);
        return ret.second ? UMA_RESULT_SUCCESS : UMA_RESULT_ERROR_UNKNOWN;
    }

    enum uma_result_t remove(const void *ptr, size_t size) {
        std::unique_lock<std::shared_mutex> lock(mtx);

        map.erase(reinterpret_cast<uintptr_t>(ptr));

        // TODO: handle removing part of the range
        (void)size;

        return UMA_RESULT_SUCCESS;
    }

    void *find(const void *ptr) {
        std::shared_lock<std::shared_mutex> lock(mtx);

        auto intptr = reinterpret_cast<uintptr_t>(ptr);
        auto it = map.upper_bound(intptr);
        if (it == map.begin()) {
            return nullptr;
        }

        --it;
        if (intptr >= it->first && intptr < it->first + it->second.first) {
            return it->second.second;
        }

        return nullptr;
    }

  private:
    std::shared_mutex mtx;
    std::map<uintptr_t, std::pair<size_t, void *>> map;
};

} // namespace uma

#endif /* UR_UMA_TRACKER_HPP */
