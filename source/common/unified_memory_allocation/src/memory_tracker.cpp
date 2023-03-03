/*
 *
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "memory_tracker_internal.h"

#include <map>
#include <mutex>
#include <shared_mutex>

// TODO: reimplement in C and optimize...
struct uma_memory_tracker_t {
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

extern "C" {

enum uma_result_t umaMemoryTrackerCreate(uma_memory_tracker_handle_t *hTracker) {
    try {
        *hTracker = new uma_memory_tracker_t;
        return UMA_RESULT_SUCCESS;
    } catch (std::bad_alloc &) {
        return UMA_RESULT_ERROR_OUT_OF_HOST_MEMORY;
    } catch (...) {
        return UMA_RESULT_ERROR_UNKNOWN;
    }
}

void umaMemoryTrackerDestroy(uma_memory_tracker_handle_t hTracker) {
    delete hTracker;
}

void *umaMemoryTrackerGetPool(uma_memory_tracker_handle_t hTracker, const void *ptr) {
    return hTracker->find(ptr);
}

enum uma_result_t umaMemoryTrackerAdd(uma_memory_tracker_handle_t hTracker, void *pool, const void *ptr, size_t size) {
    return hTracker->add(pool, ptr, size);
}

enum uma_result_t umaMemoryTrackerRemove(uma_memory_tracker_handle_t hTracker, const void *ptr, size_t size) {
    return hTracker->remove(ptr, size);
}
}
