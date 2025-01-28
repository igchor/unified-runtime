#include <layers/zel_tracing_api.h>
#include <loader/ze_loader.h>

#include <unistd.h>

#include <concepts>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <memory>
#include <cstdlib>
#include <cstring>
#include <errno.h>

#pragma once

#define ZE_CALL(call) \
do {\
auto status_ = call;\
if (status_ != ZE_RESULT_SUCCESS) { \
    std::cerr << "Error: " << #call << " " << std::hex << status_ << "\n"; \
    std::exit(1); \
}\
} while(0);

inline std::shared_ptr<_zel_tracer_handle_t>
enableTracing(zel_core_callbacks_t &&prologueCallbacks,
              zel_core_callbacks_t &&epilogueCallbacks) {
  ZE_CALL(zeInit(ZE_INIT_FLAG_GPU_ONLY));

  zel_tracer_desc_t tracer_desc = {ZEL_STRUCTURE_TYPE_TRACER_EXP_DESC, nullptr,
                                   nullptr};
  zel_tracer_handle_t tracer = nullptr;
  ZE_CALL(zelTracerCreate(&tracer_desc, &tracer));

//   ZE_CALL(zelTracerSetPrologues(tracer, &prologueCallbacks),
//             ZE_RESULT_SUCCESS);
//   ZE_CALL(zelTracerSetEpilogues(tracer, &epilogueCallbacks),
//             ZE_RESULT_SUCCESS);

  return std::shared_ptr<_zel_tracer_handle_t>(
      tracer, [](zel_tracer_handle_t tracer) { zelTracerDestroy(tracer); });
}

template <class T> std::string getZeStructureType() {
    return "TODO";
}
// Specializations for various L0 structures
template <> std::string getZeStructureType<ze_event_pool_desc_t>() {
  return "ZE_STRUCTURE_TYPE_EVENT_POOL_DESC";
}
template <> std::string getZeStructureType<ze_fence_desc_t>() {
  return "ZE_STRUCTURE_TYPE_FENCE_DESC";
}
template <> std::string getZeStructureType<ze_command_list_desc_t>() {
  return "ZE_STRUCTURE_TYPE_COMMAND_LIST_DESC";
}
template <>
std::string
getZeStructureType<ze_mutable_command_list_exp_properties_t>() {
  return "ZE_STRUCTURE_TYPE_MUTABLE_COMMAND_LIST_EXP_PROPERTIES";
}
template <>
std::string getZeStructureType<ze_mutable_command_list_exp_desc_t>() {
  return "ZE_STRUCTURE_TYPE_MUTABLE_COMMAND_LIST_EXP_DESC";
}
template <>
std::string getZeStructureType<ze_mutable_command_id_exp_desc_t>() {
  return "ZE_STRUCTURE_TYPE_MUTABLE_COMMAND_ID_EXP_DESC";
}
template <>
std::string getZeStructureType<ze_mutable_group_count_exp_desc_t>() {
  return "ZE_STRUCTURE_TYPE_MUTABLE_GROUP_COUNT_EXP_DESC";
}
template <>
std::string getZeStructureType<ze_mutable_group_size_exp_desc_t>() {
  return "ZE_STRUCTURE_TYPE_MUTABLE_GROUP_SIZE_EXP_DESC";
}
template <>
std::string getZeStructureType<ze_mutable_global_offset_exp_desc_t>() {
  return "ZE_STRUCTURE_TYPE_MUTABLE_GLOBAL_OFFSET_EXP_DESC";
}
template <>
std::string
getZeStructureType<ze_mutable_kernel_argument_exp_desc_t>() {
  return "ZE_STRUCTURE_TYPE_MUTABLE_KERNEL_ARGUMENT_EXP_DESC";
}
template <>
std::string getZeStructureType<ze_mutable_commands_exp_desc_t>() {
  return "ZE_STRUCTURE_TYPE_MUTABLE_COMMANDS_EXP_DESC";
}
template <> std::string getZeStructureType<ze_context_desc_t>() {
  return "ZE_STRUCTURE_TYPE_CONTEXT_DESC";
}
template <>
std::string
getZeStructureType<ze_relaxed_allocation_limits_exp_desc_t>() {
  return "ZE_STRUCTURE_TYPE_RELAXED_ALLOCATION_LIMITS_EXP_DESC";
}
template <>
std::string
getZeStructureType<ze_kernel_max_group_size_properties_ext_t>() {
  return "ZE_STRUCTURE_TYPE_KERNEL_MAX_GROUP_SIZE_EXT_PROPERTIES";
}
template <> std::string getZeStructureType<ze_host_mem_alloc_desc_t>() {
  return "ZE_STRUCTURE_TYPE_HOST_MEM_ALLOC_DESC";
}
template <>
std::string getZeStructureType<ze_device_mem_alloc_desc_t>() {
  return "ZE_STRUCTURE_TYPE_DEVICE_MEM_ALLOC_DESC";
}
template <> std::string getZeStructureType<ze_command_queue_desc_t>() {
  return "ZE_STRUCTURE_TYPE_COMMAND_QUEUE_DESC";
}
template <> std::string getZeStructureType<ze_image_desc_t>() {
  return "ZE_STRUCTURE_TYPE_IMAGE_DESC";
}
template <>
std::string getZeStructureType<ze_image_bindless_exp_desc_t>() {
  return "ZE_STRUCTURE_TYPE_BINDLESS_IMAGE_EXP_DESC";
}
template <>
std::string getZeStructureType<ze_image_pitched_exp_desc_t>() {
  return "ZE_STRUCTURE_TYPE_PITCHED_IMAGE_EXP_DESC";
}
template <> std::string getZeStructureType<ze_module_desc_t>() {
  return "ZE_STRUCTURE_TYPE_MODULE_DESC";
}
template <>
std::string getZeStructureType<ze_module_program_exp_desc_t>() {
  return "ZE_STRUCTURE_TYPE_MODULE_PROGRAM_EXP_DESC";
}
template <> std::string getZeStructureType<ze_kernel_desc_t>() {
  return "ZE_STRUCTURE_TYPE_KERNEL_DESC";
}
template <> std::string getZeStructureType<ze_event_desc_t>() {
  return "ZE_STRUCTURE_TYPE_EVENT_DESC";
}
template <> std::string getZeStructureType<ze_sampler_desc_t>() {
  return "ZE_STRUCTURE_TYPE_SAMPLER_DESC";
}
template <> std::string getZeStructureType<ze_physical_mem_desc_t>() {
  return "ZE_STRUCTURE_TYPE_PHYSICAL_MEM_DESC";
}
template <> std::string getZeStructureType<ze_driver_properties_t>() {
  return "ZE_STRUCTURE_TYPE_DRIVER_PROPERTIES";
}
template <> std::string getZeStructureType<ze_device_properties_t>() {
  return "ZE_STRUCTURE_TYPE_DEVICE_PROPERTIES";
}
template <>
std::string getZeStructureType<ze_device_p2p_properties_t>() {
  return "ZE_STRUCTURE_TYPE_DEVICE_P2P_PROPERTIES";
}
template <>
std::string getZeStructureType<ze_device_compute_properties_t>() {
  return "ZE_STRUCTURE_TYPE_DEVICE_COMPUTE_PROPERTIES";
}
template <>
std::string getZeStructureType<ze_command_queue_group_properties_t>() {
  return "ZE_STRUCTURE_TYPE_COMMAND_QUEUE_GROUP_PROPERTIES";
}
template <>
std::string getZeStructureType<ze_device_image_properties_t>() {
  return "ZE_STRUCTURE_TYPE_DEVICE_IMAGE_PROPERTIES";
}
template <>
std::string getZeStructureType<ze_device_module_properties_t>() {
  return "ZE_STRUCTURE_TYPE_DEVICE_MODULE_PROPERTIES";
}
template <>
std::string getZeStructureType<ze_device_cache_properties_t>() {
  return "ZE_STRUCTURE_TYPE_DEVICE_CACHE_PROPERTIES";
}
template <>
std::string getZeStructureType<ze_device_memory_properties_t>() {
  return "ZE_STRUCTURE_TYPE_DEVICE_MEMORY_PROPERTIES";
}
template <>
std::string getZeStructureType<ze_device_memory_ext_properties_t>() {
  return "ZE_STRUCTURE_TYPE_DEVICE_MEMORY_EXT_PROPERTIES";
}
template <>
std::string getZeStructureType<ze_device_ip_version_ext_t>() {
  return "ZE_STRUCTURE_TYPE_DEVICE_IP_VERSION_EXT";
}
template <>
std::string getZeStructureType<ze_device_memory_access_properties_t>() {
  return "ZE_STRUCTURE_TYPE_DEVICE_MEMORY_ACCESS_PROPERTIES";
}
template <> std::string getZeStructureType<ze_module_properties_t>() {
  return "ZE_STRUCTURE_TYPE_MODULE_PROPERTIES";
}
template <> std::string getZeStructureType<ze_kernel_properties_t>() {
  return "ZE_STRUCTURE_TYPE_KERNEL_PROPERTIES";
}
template <>
std::string getZeStructureType<ze_memory_allocation_properties_t>() {
  return "ZE_STRUCTURE_TYPE_MEMORY_ALLOCATION_PROPERTIES";
}
template <> std::string getZeStructureType<ze_pci_ext_properties_t>() {
  return "ZE_STRUCTURE_TYPE_PCI_EXT_PROPERTIES";
}
template <> std::string getZeStructureType<ze_event_pool_counter_based_exp_desc_t>() {
    return "ZE_STRUCTURE_TYPE_COUNTER_BASED_EVENT_POOL_EXP_DESC";
}

auto visit_descriptor(const ze_base_desc_t *base, auto fn) {
    if (base->stype == ZE_STRUCTURE_TYPE_EVENT_POOL_DESC) {
        return fn(*reinterpret_cast<const ze_event_pool_desc_t*>(base));
    } else if (base->stype == ZE_STRUCTURE_TYPE_FENCE_DESC) {
        return fn(*reinterpret_cast<const ze_fence_desc_t*>(base));
    } else if (base->stype == ZE_STRUCTURE_TYPE_COMMAND_LIST_DESC) {
        return fn(*reinterpret_cast<const ze_command_list_desc_t*>(base));
    } else if (base->stype == ZE_STRUCTURE_TYPE_CONTEXT_DESC) {
        return fn(*reinterpret_cast<const ze_context_desc_t*>(base));
    } else if (base->stype == ZE_STRUCTURE_TYPE_HOST_MEM_ALLOC_DESC) {
        return fn(*reinterpret_cast<const ze_host_mem_alloc_desc_t*>(base));
    } else if (base->stype == ZE_STRUCTURE_TYPE_DEVICE_MEM_ALLOC_DESC) {
        return fn(*reinterpret_cast<const ze_device_mem_alloc_desc_t*>(base));
    } else if (base->stype == ZE_STRUCTURE_TYPE_COMMAND_QUEUE_DESC) {
        return fn(*reinterpret_cast<const ze_command_queue_desc_t*>(base));
    } else if (base->stype == ZE_STRUCTURE_TYPE_IMAGE_DESC) {
        return fn(*reinterpret_cast<const ze_image_desc_t*>(base));
    } else if (base->stype == ZE_STRUCTURE_TYPE_MODULE_DESC) {
        return fn(*reinterpret_cast<const ze_module_desc_t*>(base));
    } else if (base->stype == ZE_STRUCTURE_TYPE_KERNEL_DESC) {
        return fn(*reinterpret_cast<const ze_kernel_desc_t*>(base));
    } else if (base->stype == ZE_STRUCTURE_TYPE_EVENT_DESC) {
        return fn(*reinterpret_cast<const ze_event_desc_t*>(base));
    } else if (base->stype == ZE_STRUCTURE_TYPE_SAMPLER_DESC) {
        return fn(*reinterpret_cast<const ze_sampler_desc_t*>(base));
    } else if (base->stype == ZE_STRUCTURE_TYPE_PHYSICAL_MEM_DESC) {
        return fn(*reinterpret_cast<const ze_physical_mem_desc_t*>(base));
    } else if (base->stype == ZE_STRUCTURE_TYPE_COUNTER_BASED_EVENT_POOL_EXP_DESC) {
        return fn(*reinterpret_cast<const ze_event_pool_counter_based_exp_desc_t*>(base));
    } else {
        std::cerr << "Unknown structure type: " << std::hex << base->stype << "\n";
        return std::pair<std::string, std::string>{"",""};
        // std::exit(1);
    }
}

std::string desc_type_to_string(auto desc) {
    if constexpr (std::is_same_v<decltype(desc), ze_event_pool_desc_t>) {
        return "ze_event_pool_desc_t";
    } else if constexpr (std::is_same_v<decltype(desc), ze_fence_desc_t>) {
        return "ze_fence_desc_t";
    } else if constexpr (std::is_same_v<decltype(desc), ze_command_list_desc_t>) {
        return "ze_command_list_desc_t";
    } else if constexpr (std::is_same_v<decltype(desc), ze_context_desc_t>) {
        return "ze_context_desc_t";
    } else if constexpr (std::is_same_v<decltype(desc), ze_host_mem_alloc_desc_t>) {
        return "ze_host_mem_alloc_desc_t";
    } else if constexpr (std::is_same_v<decltype(desc), ze_device_mem_alloc_desc_t>) {
        return "ze_device_mem_alloc_desc_t";
    } else if constexpr (std::is_same_v<decltype(desc), ze_command_queue_desc_t>) {
        return "ze_command_queue_desc_t";
    } else if constexpr (std::is_same_v<decltype(desc), ze_image_desc_t>) {
        return "ze_image_desc_t";
    } else if constexpr (std::is_same_v<decltype(desc), ze_module_desc_t>) {
        return "ze_module_desc_t";
    } else if constexpr (std::is_same_v<decltype(desc), ze_kernel_desc_t>) {
        return "ze_kernel_desc_t";
    } else if constexpr (std::is_same_v<decltype(desc), ze_event_desc_t>) {
        return "ze_event_desc_t";
    } else if constexpr (std::is_same_v<decltype(desc), ze_sampler_desc_t>) {
        return "ze_sampler_desc_t";
    } else if constexpr (std::is_same_v<decltype(desc), ze_physical_mem_desc_t>) {
        return "ze_physical_mem_desc_t";
    } else if constexpr (std::is_same_v<decltype(desc), ze_event_pool_counter_based_exp_desc_t>) {
        return "ze_event_pool_counter_based_exp_desc_t";
    } else {
        return "Unknown";
    }
}

template<typename T, typename ... U>
concept AnyOf = (std::same_as<T, U> || ...);

template <typename T> concept Handle = AnyOf<
T,
ze_driver_handle_t,
ze_device_handle_t,
ze_context_handle_t,
ze_command_queue_handle_t,
ze_command_list_handle_t,
ze_fence_handle_t,
ze_event_pool_handle_t,
ze_event_handle_t,
ze_image_handle_t,
ze_module_handle_t,
ze_module_build_log_handle_t,
ze_kernel_handle_t,
ze_sampler_handle_t,
ze_physical_mem_handle_t,
ze_fabric_vertex_handle_t,
ze_fabric_edge_handle_t,
ze_rtas_builder_exp_handle_t,
ze_rtas_parallel_operation_exp_handle_t
>;

// template <typename T> concept Descriptor = requires(T t) {
//     { t.stype } -> std::convertible_to<const ze_structure_type_t&>;
// };

template <typename T> concept Descriptor = requires(T t) {
    { get_assignments(t) } -> std::convertible_to<std::vector<std::string>>;
};

auto make_tuple1(auto in_struct) {
    auto [a] = in_struct;
    return std::make_tuple(a);
};
auto make_tuple2(auto in_struct) {
    auto [a,b] = in_struct;
    return std::make_tuple(a,b);
};
auto make_tuple3(auto in_struct) {
    auto [a,b,c] = in_struct;
    return std::make_tuple(a,b,c);
};
auto make_tuple4(auto in_struct) {
    auto [a,b,c,d] = in_struct;
    return std::make_tuple(a,b,c,d);
};
auto make_tuple5(auto in_struct) {
    auto [a,b,c,d,e] = in_struct;
    return std::make_tuple(a,b,c,d,e);
};
auto make_tuple6(auto in_struct) {
    auto [a,b,c,d,e,f] = in_struct;
    return std::make_tuple(a,b,c,d,e,f);
};
auto make_tuple7(auto in_struct) {
    auto [a,b,c,d,e,f,g] = in_struct;
    return std::make_tuple(a,b,c,d,e,f,g);
};
auto make_tuple8(auto in_struct) {
    auto [a,b,c,d,e,f,g,h] = in_struct;
    return std::make_tuple(a,b,c,d,e,f,g,h);
};
auto make_tuple9(auto in_struct) {
    auto [a,b,c,d,e,f,g,h,i] = in_struct;
    return std::make_tuple(a,b,c,d,e,f,g,h,i);
};
auto make_tuple10(auto in_struct) {
    auto [a,b,c,d,e,f,g,h,i,j] = in_struct;
    return std::make_tuple(a,b,c,d,e,f,g,h,i,j);
};
auto make_tuple11(auto in_struct) {
    auto [a,b,c,d,e,f,g,h,i,j,k] = in_struct;
    return std::make_tuple(a,b,c,d,e,f,g,h,i,j,k);
};
auto make_tuple12(auto in_struct) {
    auto [a,b,c,d,e,f,g,h,i,j,k,l] = in_struct;
    return std::make_tuple(a,b,c,d,e,f,g,h,i,j,k,l);
};

auto make_tuple(auto in_struct) {
    if constexpr (sizeof(in_struct) / 8 == 1) {
        return make_tuple1(in_struct);
    } else if constexpr (sizeof(in_struct) / 8 == 2) {
        return make_tuple2(in_struct);
    } else if constexpr (sizeof(in_struct) / 8 == 3) {
        return make_tuple3(in_struct);
    } else if constexpr (sizeof(in_struct) / 8 == 4) {
        return make_tuple4(in_struct);
    } else if constexpr (sizeof(in_struct) / 8 == 5) {
        return make_tuple5(in_struct);
    } else if constexpr (sizeof(in_struct) / 8 == 6) {
        return make_tuple6(in_struct);
    } else if constexpr (sizeof(in_struct) / 8 == 7) {
        return make_tuple7(in_struct);
    } else if constexpr (sizeof(in_struct) / 8 == 8) {
        return make_tuple8(in_struct);
    } else if constexpr (sizeof(in_struct) / 8 == 9) {
        return make_tuple9(in_struct);
    } else if constexpr (sizeof(in_struct) / 8 == 10) {
        return make_tuple10(in_struct);
    } else if constexpr (sizeof(in_struct) / 8 == 11) {
        return make_tuple11(in_struct);
    } else if constexpr (sizeof(in_struct) / 8 == 12) {
        return make_tuple12(in_struct);
    }
}

template <typename T> concept Pointer = requires(T t) {
    { *t } -> std::convertible_to<void*>;
};

template <typename T> concept Properties = !Descriptor<T> && requires(T t) {
    { t.stype } -> std::convertible_to<const ze_structure_type_t&>;
};

static int next_var_id = 0;
static std::unordered_map<void*, std::string> arg_to_var_name;
static std::unordered_map<void*, int64_t> arg_to_int_value;
static std::unordered_map<void*, std::string> driver_allocation_to_var_name;

std::string nextVarName(std::string_view prefix) {
    return std::string(prefix) + std::to_string(next_var_id++);
}

template <typename T>
concept Stringifiable = requires(T member) {
    { std::to_string(member) } -> std::convertible_to<std::string>;
};

struct Args {
    void addArg(auto arg) {
        // need manual fixup
        params += typeid(arg).name() + std::string("_TODO,");
    }
    void addArg(Stringifiable auto arg) {
        // need manual fixup
        params += typeid(arg).name() + std::string("_TODO=");
        params += std::to_string(arg) + ",";
    }
    void addArg(void **arg) {
        if (*arg == nullptr) {
            params += "nullptr,";
            return;
        }

        auto it = driver_allocation_to_var_name.find(*arg);
        if (it != driver_allocation_to_var_name.end()) {
            params += it->second + ",";
            return;
        }

        params += "TODO_input,";
    }
    void addArg(const void **arg) {
        if (*arg == nullptr) {
            params += "nullptr,";
            return;
        }

        auto it = driver_allocation_to_var_name.find(const_cast<void*>(*arg));
        if (it != driver_allocation_to_var_name.end()) {
            params += it->second + ",";
            return;
        }

        // if nothing is found, this can be a pointer to host memory
        // we can try to figure out size and read it, but this would
        // make the generate code huge
        params += "TODO_host_ptr,";
    }
    void addArg(void ***arg) {
        if (*arg == nullptr) {
            params += "nullptr,";
            return;
        }

        auto [it, inserted] = driver_allocation_to_var_name.try_emplace(**arg, nextVarName("ptr"));
        if (!inserted) {
            std::cerr << "Unknown allocation: " << it->second << "\n"; 
        }

        params += "&" + it->second + ",";
    }
    void addArg(std::integral auto *arg) {
        auto value = *arg;
        params += std::to_string(value) + ",";
    }
    void addArg(std::integral auto **arg) {
        if (*arg == nullptr) {
            params += "nullptr,";
            return;
        }

        if (arg_to_int_value.find((void*)(*arg)) == arg_to_int_value.end()) {
            // TODO: this might be needed for some queries
            // we can just skip this call entirely, it's most likely a query
            params += "nullptr,";
            arg_to_int_value[(void*)(*arg)] = **arg;
            return;
        } else {
            params += std::to_string(arg_to_int_value.find((void*)(*arg))->second) + ",";
        }
    }
    void addArg(Handle auto *arg) {
        if (*arg == nullptr) {
            params += "nullptr,";
            return;
        }

        if (arg_to_var_name.find((void*)(*arg)) == arg_to_var_name.end()) {
            std::cerr << "Unknown handle: " << arg_to_var_name.find((void*)(*arg))->second << "\n";
            return;
        }

        params += arg_to_var_name[(void*)(*arg)] + ",";
    }
    void addArg(Handle auto **arg) {
        if (*arg == nullptr) {
            params += "nullptr,";
            return;
        }

        if (arg_to_var_name.find((void*)(**arg)) != arg_to_var_name.end()) {
            // if the handle is known, it means it's a pointer to array of handles
            // TODO: implement
            params += "TODO_array_of_handles,";
            return;
        }
        // otherwise it's output parameter
        auto varName = nextVarName("handle");
        init += "\nvoid* " + varName + "{};";
        arg_to_var_name[(void*)(**arg)] = varName;
        params += "&" + varName + ",";
    }
    void addArg(Properties auto **arg) {
        params += "TODOproperties,";
    }
    void addArg(Descriptor auto **arg) {
        auto [var, in] = initDescriptor(**arg);
        init += in;
        params += "&" + var + ",";
    }

    std::string init;
    std::string params;

private:
    std::pair<std::string, std::string> initDescriptor(Descriptor auto desc) {
        std::string local_var, next_var;
        std::string local_init, next_init;
        if (desc.pNext != nullptr) {
            std::tie(next_var, next_init) =
                visit_descriptor(reinterpret_cast<const ze_base_desc_t*>(desc.pNext), [&](auto next) {
                    return initDescriptor(next);
                });
        }

        local_init += next_init;

        local_var = nextVarName("desc");
        local_init += "\n" + desc_type_to_string(desc) + " " +  local_var + "{};";
        for (auto &v : get_assignments(desc)) {
            local_init += "\n" + local_var + "." + v + ";";
        }
        if (desc.pNext != nullptr)
            local_init += "\n" + local_var + ".pNext = &" + next_var + ";";

        return {local_var, local_init};
    }
};

Args make_args(auto params) {
    Args args;

    auto tuple = make_tuple(params);

    std::apply([&](auto... a) {
        (args.addArg(a), ...);
    }, tuple);

    return args;
}

std::string desc_member_to_string(auto member) {
    return "TODO";
}

template <Descriptor D>
std::string desc_member_to_string(D member) {
    return getZeStructureType<D>();
}

std::string desc_member_to_string(Stringifiable auto member) {
    return std::to_string(member);
}

// std::stringstream os;

void ze_cb_invoke(std::string_view functionName, Args&& args, ze_result_t result) {
    std::ignore = result;

    std::cout << args.init << "\n";
    std::cout << "ZE_CALL(" << functionName << "(";
    args.params.pop_back(); // remove trailing comma
    std::cout << args.params;
    std::cout << "))\n";
}

