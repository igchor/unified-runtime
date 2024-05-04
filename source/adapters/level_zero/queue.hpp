//===--------- queue.hpp - Level Zero Adapter -----------------------------===//
//
// Copyright (C) 2023 Intel Corporation
//
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM
// Exceptions. See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#pragma once

#include <cassert>
#include <list>
#include <map>
#include <optional>
#include <queue>
#include <stdarg.h>
#include <string>
#include <unordered_map>
#include <vector>

#include <ur/ur.hpp>
#include <ur_api.h>
#include <ze_api.h>
#include <zes_api.h>

#include "common.hpp"
#include "device.hpp"

extern "C" {
ur_result_t urQueueReleaseInternal(ur_queue_handle_t Queue);
} // extern "C"

struct ur_completion_batch;
using ur_completion_batch_list = std::list<ur_completion_batch>;
using ur_completion_batch_it = ur_completion_batch_list::iterator;

// Event completion batch for aggregating status checks of many events into
// a single one through a barrier. Batches can be continuously reused.
// Batches start empty and accumulate events, get sealed (which issues
// an asynchronous barrier on the command list), and then must be waited on
// for all the events to complete.
struct ur_completion_batch {
  ur_completion_batch();
  ~ur_completion_batch();

  enum state {
    EMPTY,        // use() -> ACCUMULATING
    ACCUMULATING, // append() -> full -> seal() -> SEALED
    SEALED,       // checkComplete() -> COMPLETED
    COMPLETED,    // reset() -> EMPTY
  };

  // Returns the state of the batch. Might be stale.
  state getState();

  // Return the most up-to-date state of the batch. Might query the state of the
  // underlying barrier event.
  state queryState();

  // Must be called on any completion batch prior to being used for events.
  void use();

  // Checks whether the batch is at capacity. This is a soft limit and can be
  // exceeded if necessary.
  bool isFull();

  // Appends an event to the batch.
  void append();

  // Seals the event batch and appends a barrier to the command list.
  // Adding any further events after this, but before reset, is undefined.
  ur_result_t seal(ur_queue_handle_t queue, ze_command_list_handle_t cmdlist);

  // Resets a complete batch back to an empty state. Cleanups internal state
  // but keeps allocated resources for reuse.
  ur_result_t reset();

private:
  // Checks whether all the events in the batch have completed. Might query
  // the underlying event status. Can only be called on a sealed batch.
  bool checkComplete();

  // Internal barrier event that is signaled on completion of the batched
  // events.
  ur_event_handle_t barrierEvent;

  // Current batch state. Don't use directly.
  state st;

  // Number of accumulated events.
  size_t numEvents;
};

// A collection of event completion batches. Manages querying event status
// in batches of events instead of individually, reducing the number of total
// queries necessary to determine whether a set of events have signaled.
struct ur_completion_batches {
  // This structure should never be copied because it contains a stable iterator
  // into a list. Copying it would likely result in unexpected behavior.
  ur_completion_batches(const ur_completion_batches &) = delete;
  ur_completion_batches &operator=(const ur_completion_batches &) = delete;
  ur_completion_batches(ur_completion_batches &&) = default;
  ur_completion_batches &operator=(ur_completion_batches &&) = default;

  ur_completion_batches();

  // Cleans up completed batches, and, if the currently active batch
  // is full, attempts to find an empty batch to be used as active.
  // If one is found, the current one is sealed, and the new one is
  // set as active. Otherwise, UR_RESULT_ERROR_OUT_OF_RESOURCES is
  // returned to indicate that there are no batches available.
  // This is safe, but will increase how many events are associated
  // with the active batch.
  ur_result_t tryCleanup(ur_queue_handle_t queue,
                         ze_command_list_handle_t cmdlist,
                         std::vector<ur_event_handle_t> &EventList,
                         std::vector<ur_event_handle_t> &EventListToCleanup);

  // Adds an event to the the active batch.
  // Ideally, all events that are appended here are then provided in the
  // vector for cleanup. Otherwise the event batch will simply ignore
  // missing events when it comes time for cleanup.
  void append(ur_event_handle_t event);

  // Resets all the batches without waiting for event completion.
  // Only safe when the command list was fully synchronized through
  // other means.
  void forceReset();

private:
  // Checks the state of all previously sealed batches. If any are complete,
  // moves the associated events from the EventList to EventListToCleanup,
  // and then resets the batch for reuse.
  ur_result_t cleanup(std::vector<ur_event_handle_t> &EventList,
                      std::vector<ur_event_handle_t> &EventListToCleanup);

  // Moves the completed events from EventList to EventListToCleanup.
  void moveCompletedEvents(ur_completion_batch_it it,
                           std::vector<ur_event_handle_t> &EventList,
                           std::vector<ur_event_handle_t> &EventListToCleanup);

  // Find or creates an empty batch. This might fail if there are now empty
  // batches and a batch limit has been reached.
  std::optional<ur_completion_batch_it> findFirstEmptyBatchOrCreate();

  ur_completion_batch_list batches;
  std::queue<ur_completion_batch_it> sealed;
  ur_completion_batch_it active;
};

static const std::optional<bool> useDriverCounterBasedEvents = [] {
  const char *UrRet = std::getenv("UR_L0_USE_DRIVER_COUNTER_BASED_EVENTS");
  if (!UrRet) {
    return std::nullopt;
  }

  return std::make_optional(std::atoi(UrRet) != 0);
}();

ur_result_t resetCommandLists(ur_queue_handle_t Queue);
ur_result_t
CleanupEventsInImmCmdLists(ur_queue_handle_t UrQueue, bool QueueLocked = false,
                           bool QueueSynced = false,
                           ur_event_handle_t CompletedEvent = nullptr);

// Structure describing the specific use of a command-list in a queue.
// This is because command-lists are re-used across multiple queues
// in the same context.
struct ur_command_list_info_t {
  ur_command_list_info_t(ze_fence_handle_t ZeFence, bool ZeFenceInUse,
                         bool IsClosed, ze_command_queue_handle_t ZeQueue,
                         ZeStruct<ze_command_queue_desc_t> ZeQueueDesc,
                         bool UseCompletionBatching, bool CanReuse = true,
                         bool IsInOrderList = false)
      : ZeFence(ZeFence), ZeFenceInUse(ZeFenceInUse), IsClosed(IsClosed),
        ZeQueue(ZeQueue), ZeQueueDesc(ZeQueueDesc),
        IsInOrderList(IsInOrderList), CanReuse(CanReuse) {
    if (UseCompletionBatching) {
      completions = ur_completion_batches();
    }
  }
  // The Level-Zero fence that will be signalled at completion.
  // Immediate commandlists do not have an associated fence.
  // A nullptr for the fence indicates that this is an immediate commandlist.
  ze_fence_handle_t ZeFence;
  // Record if the fence is in use.
  // This is needed to avoid leak of the tracked command-list if the fence
  // was not yet signaled at the time all events in that list were already
  // completed (we are polling the fence at events completion). The fence
  // may be still "in-use" due to sporadic delay in HW.
  bool ZeFenceInUse;

  // Indicates if command list is in closed state. This is needed to avoid
  // appending commands to the closed command list.
  bool IsClosed;

  // Record the queue to which the command list will be submitted.
  ze_command_queue_handle_t ZeQueue;

  // Record the queue descriptor fields used when creating the command list
  // because we cannot recover these fields from the command list. Immediate
  // command lists are recycled across queues and then all fields are used. For
  // standard command lists only the ordinal is used. For queues created through
  // the make_queue API the descriptor is unavailable so a dummy descriptor is
  // used and then this entry is marked as not eligible for recycling.
  ZeStruct<ze_command_queue_desc_t> ZeQueueDesc;
  // Indicates if this is an inorder list
  bool IsInOrderList;
  bool CanReuse;

  // Helper functions to tell if this is a copy command-list.
  bool isCopy(ur_queue_handle_t Queue) const;

  // An optional event completion batching mechanism for out-of-order immediate
  // command lists.
  std::optional<ur_completion_batches> completions;

  // Keeps events created by commands submitted into this command-list.
  // TODO: use this for explicit wait/cleanup of events at command-list
  // completion.
  // TODO: use this for optimizing events in the same command-list, e.g.
  // only have last one visible to the host.
  std::vector<ur_event_handle_t> EventList;
  size_t size() const { return EventList.size(); }
  void append(ur_event_handle_t Event);
};

// The map type that would track all command-lists in a queue.
using ur_command_list_map_t =
    std::unordered_map<ze_command_list_handle_t, ur_command_list_info_t>;
// The iterator pointing to a specific command-list in use.
using ur_command_list_ptr_t = ur_command_list_map_t::iterator;

struct ur_immediate_command_list_t {
  ze_command_list_handle_t ZeHandle;
  ZeStruct<ze_command_queue_desc_t> ZeQueueDesc;
};

struct ur_command_queue_t {
  ze_command_queue_handle_t ZeHandle;
  ZeStruct<ze_command_queue_desc_t> ZeQueueDesc;
};

struct ur_batched_command_list_t {
  ze_command_list_handle_t ZeHandle;
  uint32_t ZeOrdinal;
};

template <typename HandleType>
struct ur_queue_group_generic {
  std::vector<HandleType> ComputeLists;
  std::vector<HandleType> MainCopyList;
  std::vector<HandleType> LinkCopyLists;
};

enum class CommandListType {
    Compute, Copy
};

template <typename HandleType>
struct ur_queue_handle_pool_generic {
  ur_command_list_pool_t(ur_queue_group_generic<HandleType>&& QueueGroup): QueueGroup(std::move(QueueGroup)) {}

  HandleType getHandle(CommandListType Type) {
    if (Type == CommandListType::Compute || (QueueGroup.MainCopyList.empty() && QueueGroup.LinkCopyLists.empty())) {
      if (QueueGroup.ComputeLists.size() == 1) // fast path for single element
        return QueueGroup.ComputeLists.front();
    
      return QueueGroup.ComputeLists[ComputeSlot.fetch_add(1, std::memory_order_acq_rel)];
    }

    auto CopyIndex = CopySlot.fetch_add(1, std::memory_order_acq_rel);
    auto NumCopyLists = QueueGroup.MainCopyList.size() + QueueGroup.LinkCopyLists.size();

    if (QueueGroup.MainCopyList.size() && CopyIndex == 0) {
      return QueueGroup.MainCopyList.front();
    }

    return QueueGroup.LinkCopyLists[(CopyIndex - QueueGroup.MainCopyList.size()) % NumCopyLists];
  }
private:
  ur_queue_group_generic<HandleType> QueueGroup;
  std::atomic<uint64_t> ComputeSlot{0};
  std::atomic<uint64_t> CopySlot{0};
};

using ur_immediate_cmdlist_pool_t = ur_queue_handle_pool_generic<ur_immediate_command_list_t>;
using ur_command_queue_pool_t = ur_queue_handle_pool_generic<ur_command_queue_t>;

// Priority of the UR queue - 1:1 mapping with L0 command queue
enum class ur_queue_priority_t {
  NORMAL = ZE_COMMAND_QUEUE_PRIORITY_NORMAL,
  LOW = ZE_COMMAND_QUEUE_PRIORITY_PRIORITY_LOW,
  HIGH = ZE_COMMAND_QUEUE_PRIORITY_PRIORITY_HIGH
};

struct ur_queue_properties_internal_t {
  ur_queue_properties_internal_t(ur_queue_flags_t Flags, bool OwnZeCommandQueue, bool UsingImmCmdLists, EventsScope ZeEventsScope, bool CounterBasedEventsEnabled, all_queue_groups_t&& QueueGroup)
      : Flags(Flags), OwnZeCommandQueue(OwnZeCommandQueue), ZeEventsScope(ZeEventsScope), CounterBasedEventsEnabled(CounterBasedEventsEnabled), QueueGroup(std::move(QueueGroup)) { }

  // Returns true if the queue is a in-order queue.
  bool isInOrderQueue() const;

  // Returns true if the queue has discard events property.
  bool isDiscardEvents() const;

  // Returns priority of the queue
  ur_queue_priority_t getPriority() const;

  // Returns true if the queue has an explicitly selected submission mode.
  bool isBatchedSubmission() const;
  bool isImmediateSubmission() const;

// TODO
// private:
  // Flags passed at the queue creation time
  ur_queue_flags_t Flags;

  // Indicates if we own the ZeCommandQueue or it came from interop that
  // asked to not transfer the ownership to SYCL RT.
  bool OwnZeCommandQueue;

  // Scope of events used for events on the queue
  // Can be adjusted with UR_L0_DEVICE_SCOPE_EVENTS
  // for non-immediate command lists
  EventsScope ZeEventsScope;

  // Keeps track of whether we are using Counter-based Events
  bool CounterBasedEventsEnabled;

  // Queue Group information
  all_queue_groups_t QueueGroupInfo;
};

struct ur_immediate_queue_t {
    ur_immediate_command_list_t getCommandList(
    CommandListType Type, bool AllowBatching,
    ze_command_queue_handle_t *ForcedCmdQueue) {
      return CmdListPool.getHandle(Type);
    }

private:
  ur_queue_handle_pool_generic<ur_immediate_command_list_t> CmdListPool;
};

struct ur_batched_queue_t {
  ur_batched_command_list_t getCommandList(
    CommandListType Type, bool AllowBatching,
    ze_command_queue_handle_t *ForcedCmdQueue) {
      auto OpenCommandList = getOpenCommandList(Type);
      if (OpenCommandList.has_value() && AllowBatching) {
        return OpenCommandList.value();
      } else if (OpenCommandList.has_value()) {
        // TODO: should we execute what's there or just select another command list?
        // or maybe we can just use immediate command list for this case?
        if (auto Res = Queue->executeOpenCommandList(UseCopyEngine))
          return Res;
      }

      // TODO: look through the list of command lists and find one that has been signaled

      // If there are no available command lists nor signalled command lists,
  // then we must create another command list.
  ur_result = Queue->createCommandList(UseCopyEngine, CommandList);
  CommandList->second.ZeFenceInUse = true;
  return ur_result;


  }

private:
  ur_command_queue_pool_t QueuePool;
};

struct ur_queue_handle_t_ : _ur_object {
  ur_queue_handle_t_(ur_context_handle_t Context, ur_device_handle_t Device, ur_queue_properties_internal_t Properties
                     ) : Context{Context}, Device{Device},
      Properties(Properties) {

    if (Properties.isImmediateSubmission()) {
      QueueHandlePool = std::make_unique<ur_queue_handle_pool_generic<ur_immediate_command_list_t>>(createImmediateCommandListsGroup(Context, Device, Properties));
    } else {
      QueueHandlePool = std::make_unique<ur_queue_handle_pool_generic<ur_command_queue_t>>(createZeQueueGroup(Context, Device, Properties));
    }
}
  // Queue properties
  ur_queue_properties_internal_t Properties;

  // Pool of handles for command lists or (L0) command queues.
  std::unique_ptr<ur_queue_handle_pool_t> QueueHandlePool;

  // Keeps the PI context to which this queue belongs.
  // This field is only set at ur_queue_handle_t creation time, and cannot
  // change. Therefore it can be accessed without holding a lock on this
  // ur_queue_handle_t.
  const ur_context_handle_t Context;

  // Keeps the PI device to which this queue belongs.
  // This field is only set at ur_queue_handle_t creation time, and cannot
  // change. Therefore it can be accessed without holding a lock on this
  // ur_queue_handle_t.
  const ur_device_handle_t Device;

  // Keeps track of the event associated with the last enqueued command into
  // this queue. this is used to add dependency with the last command to add
  // in-order semantics and updated with the latest event each time a new
  // command is enqueued.
  ur_event_handle_t LastCommandEvent = nullptr;

  // Helper data structure to hold all variables related to batching
  struct command_batch {
    // These two members are used to keep track of how often the
    // batching closes and executes a command list before reaching the
    // QueueComputeBatchSize limit, versus how often we reach the limit.
    // This info might be used to vary the QueueComputeBatchSize value.
    uint32_t NumTimesClosedEarly = {0};
    uint32_t NumTimesClosedFull = {0};

    // Open command list fields for batching commands into this queue.
    ur_command_list_ptr_t OpenCommandList{};

    // Approximate number of commands that are allowed to be batched for
    // this queue.
    // Added this member to the queue rather than using a global variable
    // so that future implementation could use heuristics to change this on
    // a queue specific basis. And by putting it in the queue itself, this
    // is thread safe because of the locking of the queue that occurs.
    uint32_t QueueBatchSize = {0};
  };

  // ComputeCommandBatch holds data related to batching of non-copy commands.
  // CopyCommandBatch holds data related to batching of copy commands.
  command_batch ComputeCommandBatch, CopyCommandBatch;

  // A helper structure to keep active barriers of the queue.
  // It additionally manages ref-count of events in this list.
  struct active_barriers {
    std::vector<ur_event_handle_t> Events;
    void add(ur_event_handle_t &Event);
    ur_result_t clear();
    bool empty() { return Events.empty(); }
    std::vector<ur_event_handle_t> &vector() { return Events; }
  };
  // A collection of currently active barriers.
  // These should be inserted into a command list whenever an available command
  // list is needed for a command.
  active_barriers ActiveBarriers;

  // Besides each PI object keeping a total reference count in
  // _ur_object::RefCount we keep special track of the queue *external*
  // references. This way we are able to tell when the queue is being finished
  // externally, and can wait for internal references to complete, and do proper
  // cleanup of the queue.
  // This counter doesn't track the lifetime of a queue object, it only tracks
  // the number of external references. I.e. even if it reaches zero a queue
  // object may not be destroyed and can be used internally in the plugin.
  // That's why we intentionally don't use atomic type for this counter to
  // enforce guarding with a mutex all the work involving this counter.
  uint32_t RefCountExternal{1};

  // Indicates that the queue is healthy and all operations on it are OK.
  bool Healthy{true};

  // The following data structures and methods are used only for handling
  // in-order queue with discard_events property. Some commands in such queue
  // may have discarded event. Which means that event is not visible outside of
  // the plugin. It is possible to reset and reuse discarded events in the same
  // in-order queue because of the dependency between commands. We don't have to
  // wait event completion to do this. We use the following 2-event model to
  // reuse events inside each command list:
  //
  // Operation1 = zeCommantListAppendMemoryCopy (signal ze_event1)
  // zeCommandListAppendBarrier(wait for ze_event1)
  // zeCommandListAppendEventReset(ze_event1)
  // # Create new ur_event_handle_t using ze_event1 and append to the cache.
  //
  // Operation2 = zeCommandListAppendMemoryCopy (signal ze_event2)
  // zeCommandListAppendBarrier(wait for ze_event2)
  // zeCommandListAppendEventReset(ze_event2)
  // # Create new ur_event_handle_t using ze_event2 and append to the cache.
  //
  // # Get ur_event_handle_t from the beginning of the cache because there are
  // two events there. So it is guaranteed that we do round-robin between two
  // # events - event from the last command is appended to the cache.
  // Operation3 = zeCommandListAppendMemoryCopy (signal ze_event1)
  // # The same ze_event1 is used for Operation1 and Operation3.
  //
  // When we switch to a different command list we need to signal new event and
  // wait for it in the new command list using barrier.
  // [CmdList1]
  // Operation1 = zeCommantListAppendMemoryCopy (signal event1)
  // zeCommandListAppendBarrier(wait for event1)
  // zeCommandListAppendEventReset(event1)
  // zeCommandListAppendSignalEvent(NewEvent)
  //
  // [CmdList2]
  // zeCommandListAppendBarrier(wait for NewEvent)
  //
  // This barrier guarantees that command list execution starts only after
  // completion of previous command list which signals aforementioned event. It
  // allows to reset and reuse same event handles inside all command lists in
  // scope of the queue. It means that we need 2 reusable events of each type
  // (host-visible and device-scope) per queue at maximum.

  // This data member keeps track of the last used command list and allows to
  // handle switch of immediate command lists because immediate command lists
  // are never closed unlike regular command lists.
  ur_command_list_ptr_t LastUsedCommandList = CommandListMap.end();

  // Vector of 2 lists of reusable events: host-visible and device-scope.
  // They are separated to allow faster access to stored events depending on
  // requested type of event. Each list contains events which can be reused
  // inside all command lists in the queue as described in the 2-event model.
  // Leftover events in the cache are relased at the queue destruction.
  std::vector<std::list<ur_event_handle_t>> EventCaches{2};
  std::vector<std::unordered_map<ur_device_handle_t, size_t>>
      EventCachesDeviceMap{2};

  // adjust the queue's batch size, knowing that the current command list
  // is being closed with a full batch.
  // For copy commands, IsCopy is set to 'true'.
  // For non-copy commands, IsCopy is set to 'false'.
  void adjustBatchSizeForFullBatch(bool IsCopy);

  // adjust the queue's batch size, knowing that the current command list
  // is being closed with only a partial batch of commands.
  // For copy commands, IsCopy is set to 'true'.
  // For non-copy commands, IsCopy is set to 'false'.
  void adjustBatchSizeForPartialBatch(bool IsCopy);

  // Attach a command list to this queue.
  // For non-immediate commandlist also close and execute it.
  // Note that this command list cannot be appended to after this.
  // The "IsBlocking" tells if the wait for completion is required.
  // If OKToBatchCommand is true, then this command list may be executed
  // immediately, or it may be left open for other future command to be
  // batched into.
  // If IsBlocking is true, then batching will not be allowed regardless
  // of the value of OKToBatchCommand
  //
  // For immediate commandlists, no close and execute is necessary.
  ur_result_t executeCommandList(ur_command_list_ptr_t CommandList,
                                 bool IsBlocking = false,
                                 bool OKToBatchCommand = false);

  // Helper method telling whether we need to reuse discarded event in this
  // queue.
  bool doReuseDiscardedEvents();

  // Append command to provided command list to wait and reset the last event if
  // it is discarded and create new ur_event_handle_t wrapper using the same
  // native event and put it to the cache. We call this method after each
  // command submission to make native event available to use by next commands.
  ur_result_t resetDiscardedEvent(ur_command_list_ptr_t);

  // Put ur_event_handle_t to the cache. Provided ur_event_handle_t object is
  // not used by any command but its ZeEvent is used by many ur_event_handle_t
  // objects. Commands to wait and reset ZeEvent must be submitted to the queue
  // before calling this method.
  ur_result_t addEventToQueueCache(ur_event_handle_t Event);

  // Returns true if any commands for this queue are allowed to
  // be batched together.
  // For copy commands, IsCopy is set to 'true'.
  // For non-copy commands, IsCopy is set to 'false'.
  bool isBatchingAllowed(bool IsCopy) const;

  // Wait for all commandlists associated with this Queue to finish operations.
  [[nodiscard]] ur_result_t synchronize();

  // Get event from the queue's cache.
  // Returns nullptr if the cache doesn't contain any reusable events or if the
  // cache contains only one event which corresponds to the previous command and
  // can't be used for the current command because we can't use the same event
  // two times in a row and have to do round-robin between two events. Otherwise
  // it picks an event from the beginning of the cache and returns it. Event
  // from the last command is always appended to the end of the list.
  ur_event_handle_t getEventFromQueueCache(bool IsMultiDevice,
                                           bool HostVisible);

  // Returns true if an OpenCommandList has commands that need to be submitted.
  // If IsCopy is 'true', then the OpenCommandList containing copy commands is
  // checked. Otherwise, the OpenCommandList containing compute commands is
  // checked.
  bool hasOpenCommandList(bool IsCopy) const {
    const auto &CommandBatch =
        (IsCopy) ? CopyCommandBatch : ComputeCommandBatch;
    return CommandBatch.OpenCommandList != CommandListMap.end();
  }

  // Update map of memory references made by the kernels about to be submitted
  void CaptureIndirectAccesses();

  // Kernel is not necessarily submitted for execution during
  // urEnqueueKernelLaunch, it may be batched. That's why we need to save the
  // list of kernels which is going to be submitted but have not been submitted
  // yet. This is needed to capture memory allocations for each kernel with
  // indirect access in the list at the moment when kernel is really submitted
  // for execution.
  std::vector<ur_kernel_handle_t> KernelsToBeSubmitted;

  // Append command to the command list to signal new event if the last event in
  // the command list is discarded. While we submit commands in scope of the
  // same command list we can reset and reuse events but when we switch to a
  // different command list we currently need to signal new event and wait for
  // it in the new command list using barrier.
  ur_result_t signalEventFromCmdListIfLastEventDiscarded(ur_command_list_ptr_t);

  // If there is an open command list associated with this queue,
  // close it, execute it, and reset the corresponding OpenCommandList.
  // If IsCopy is 'true', then the OpenCommandList containing copy commands is
  // executed. Otherwise OpenCommandList containing compute commands is
  // executed.
  ur_result_t executeOpenCommandList(bool IsCopy);

  // Wrapper function to execute both OpenCommandLists (Copy and Compute).
  // This wrapper is helpful when all 'open' commands need to be executed.
  // Call-sites instances: urQuueueFinish, urQueueRelease, etc.
  ur_result_t executeAllOpenCommandLists() {
    using IsCopy = bool;
    if (auto Res = executeOpenCommandList(IsCopy{false}))
      return Res;
    if (auto Res = executeOpenCommandList(IsCopy{true}))
      return Res;
    return UR_RESULT_SUCCESS;
  }

  /// @brief Resets the command list and associated fence in the map and removes
  /// events from the command list.
  /// @param CommandList The caller must verify that this command list and fence
  /// have been signalled.
  /// @param MakeAvailable If the reset command list should be made available,
  /// then MakeAvailable needs to be set to true.
  /// @param EventListToCleanup  The EventListToCleanup contains a list of
  /// events from the command list which need to be cleaned up.
  /// @param CheckStatus Hint informing whether we need to check status of the
  /// events before removing them from the immediate command list. This is
  /// needed because immediate command lists are not associated with fences and
  /// in general status of the event needs to be checked.
  /// @return PI_SUCCESS if successful, PI error code otherwise.
  ur_result_t
  resetCommandList(ur_command_list_ptr_t CommandList, bool MakeAvailable,
                   std::vector<ur_event_handle_t> &EventListToCleanup,
                   bool CheckStatus = true);

  // Gets the open command containing the event, or CommandListMap.end()
  ur_command_list_ptr_t eventOpenCommandList(ur_event_handle_t Event);

  // Helper function to create a new command-list to this queue and associated
  // fence tracking its completion. This command list & fence are added to the
  // map of command lists in this queue with ZeFenceInUse = false.
  // The caller must hold a lock of the queue already.
  ur_result_t
  createCommandList(bool UseCopyEngine, ur_command_list_ptr_t &CommandList,
                    ze_command_queue_handle_t *ForcedCmdQueue = nullptr);

  // Inserts a barrier waiting for all unfinished events in ActiveBarriers into
  // CmdList. Any finished events will be removed from ActiveBarriers.
  ur_result_t insertActiveBarriers(ur_command_list_ptr_t &CmdList,
                                   bool UseCopyEngine);

  // This function considers multiple factors including copy engine
  // availability and user preference and returns a boolean that is used to
  // specify if copy engine will eventually be used for a particular command.
  bool useCopyEngine(bool PreferCopyEngine = true) const;

  // Insert a barrier waiting for the last command event into the beginning of
  // command list. This barrier guarantees that command list execution starts
  // only after completion of previous command list which signals aforementioned
  // event. It allows to reset and reuse same event handles inside all command
  // lists in the queue.
  ur_result_t
  insertStartBarrierIfDiscardEventsMode(ur_command_list_ptr_t &CmdList);

  // returns true if queue has profiling enabled
  bool isProfilingEnabled() {
    return ((this->Properties & UR_QUEUE_FLAG_PROFILING_ENABLE) != 0);
  }

  // Checks whether this queue supports and uses event completion batching.
  // Can be true only when using out-of-order immediate command lists.
  bool useCompletionBatching();

  // Threshold for cleaning up the EventList for immediate command lists.
  size_t getImmdCmmdListsEventCleanupThreshold();
};

// This helper function creates a ur_event_handle_t and associate a
// ur_queue_handle_t. Note that the caller of this function must have acquired
// lock on the Queue that is passed in.
// \param Queue ur_queue_handle_t to associate with a new event.
// \param Event a pointer to hold the newly created ur_event_handle_t
// \param CommandType various command type determined by the caller
// \param CommandList is the command list where the event is added
// \param IsInternal tells if the event is internal, i.e. visible in the L0
//        plugin only.
// \param IsMultiDevice Indicates that this event must be visible by
//        multiple devices.
// \param ForceHostVisible tells if the event must be created in
//        the host-visible pool
ur_result_t createEventAndAssociateQueue(
    ur_queue_handle_t Queue, ur_event_handle_t *Event, ur_command_t CommandType,
    ur_command_list_ptr_t CommandList, bool IsInternal, bool IsMultiDevice,
    std::optional<bool> HostVisible = std::nullopt);

// Helper function to perform the necessary cleanup of the events from reset cmd
// list.
ur_result_t CleanupEventListFromResetCmdList(
    std::vector<ur_event_handle_t> &EventListToCleanup,
    bool QueueLocked = false);
