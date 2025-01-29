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
#include <fstream>
#include <sstream>
#include <regex>
#include <string>

#pragma once

#define ZE_CALL(call) \
do {\
auto status_ = call;\
if (status_ != ZE_RESULT_SUCCESS) { \
    std::cerr << "Error: " << #call << " " << std::hex << status_ << "\n"; \
    std::exit(1); \
}\
} while(0);

inline std::string type_name_to_var_name(std::string str) {
    str = std::regex_replace(str, std::regex("\\*"), "");
    auto skip_ze = str.find('_') + 1;
    return str.substr(skip_ze, str.find_last_of('_') - skip_ze);
}

inline std::string trim_ptr(std::string str) {
    auto pos = str.find('*');
    if (pos == std::string::npos) return str;
    str.replace(pos, 1, "");
    return str;
}

inline std::string trim(std::string str)
{
    str.erase(str.find_last_not_of(", \n")+1);
    str.erase(0, str.find_first_not_of(", \n"));
    auto decayed = std::regex_replace(str, std::regex("const "), "");
    return decayed;
    // return std::regex_replace(decayed, std::regex("*"), "");
}

struct ZeApi {
    ZeApi() {
        std::ifstream file("ze_api.h");
        std::stringstream buffer;
        buffer << file.rdbuf();
        ze_api_content = buffer.str();
    }

    std::pair<std::string, std::string> get_typename_and_description(std::string fname, size_t param_idx) {
        size_t line_start_pos = ze_api_content.find(fname + "(");
        if (line_start_pos == std::string::npos) {
            throw std::runtime_error("Function " + fname + " not found");
        }
        for (size_t i = 0; i <= param_idx; i++) {
            line_start_pos = ze_api_content.find('\n', line_start_pos + 1);
            // if line contains only comment, skip it
            while (ze_api_content.find_first_of("abcdefghijklmnopqrstuvwxyz", line_start_pos + 1) >= ze_api_content.find_first_of("<", line_start_pos + 1)) {
                line_start_pos = ze_api_content.find('\n', line_start_pos + 1);
            }
        }

        // find the start of the line
        while (ze_api_content[line_start_pos] != '\n') {
            line_start_pos--;
        }
        size_t comment_start_pos = ze_api_content.find("///<", line_start_pos);
        auto type_name_and_param_name = trim(ze_api_content.substr(line_start_pos, comment_start_pos - line_start_pos));
        auto description = trim(ze_api_content.substr(comment_start_pos, ze_api_content.find('\n', comment_start_pos) - comment_start_pos));
        auto type_name = type_name_and_param_name.substr(0, type_name_and_param_name.find(' '));
        // auto param_name = type_name_and_param_name.substr(type_name_and_param_name.find(' ') + 1);
        return std::make_pair(type_name, description);
    }

    std::string get_stype(size_t value) {
        std::stringstream ss;
        ss << std::hex << value;
        std::string hex_val = "0x" + ss.str();
        
        size_t def_pos = ze_api_content.find("_ze_structure_type_t");
        size_t value_pos = ze_api_content.find(hex_val.data(), def_pos);
        size_t def_end = ze_api_content.find("} ze_structure_type_t;", def_pos);

        if (value_pos > def_end) return std::to_string(value);

        size_t line_start_pos = value_pos;

        // find the start of the line
        while (ze_api_content[line_start_pos - 1] != '\n') {
            line_start_pos--;
        }

        auto stype_name_end = ze_api_content.find('=', line_start_pos);
        return trim(ze_api_content.substr(line_start_pos, stype_name_end - line_start_pos));
    }

    std::string ze_api_content;
};

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

static ZeApi zeApi;

std::string getZeStructureType(size_t stypeValue) {
    return zeApi.get_stype(stypeValue);
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

static std::unordered_map<std::string, int> next_var_id;
static std::unordered_map<void*, std::string> arg_to_var_name;
static std::unordered_map<void*, int64_t> arg_to_int_value;
static std::unordered_map<void*, std::string> driver_allocation_to_var_name;
static std::unordered_map<void*, std::vector<void*>> array_to_handles;

std::string nextVarName(std::string_view prefix) {
    return std::string(prefix) + std::to_string(next_var_id[std::string(prefix)]++);
}

template <typename T>
concept Stringifiable = requires(T member) {
    { std::to_string(member) } -> std::convertible_to<std::string>;
};

struct Args {
    Args(std::string fname) : fname(fname) {
    }

    void addArg(auto arg, int param_index) {
        // need manual fixup
        params += typeid(arg).name() + std::string("_TODO,");
    }
    void addArg(Stringifiable auto arg, int param_index) {
        // need manual fixup
        params += typeid(arg).name() + std::string("_TODO=");
        params += std::to_string(arg) + ",";
    }
    void addArg(const ze_group_count_t **arg, int param_index) {
        auto varName = nextVarName("group_count");
        init += "\nze_group_count_t " + varName + "{";
        init += std::to_string((*arg)->groupCountX) + ",";
        init += std::to_string((*arg)->groupCountY) + ",";
        init += std::to_string((*arg)->groupCountZ);
        init += "};";
        params += "&" + varName + ",";
    }
    void addArg(void **arg, int param_index) {
        if (*arg == nullptr) {
            params += "nullptr,";
            return;
        }

        auto it = driver_allocation_to_var_name.find(*arg);
        if (it != driver_allocation_to_var_name.end()) {
            params += it->second + ",";
            return;
        }

        if ((uintptr_t) *arg > 0x700000000000 && (uintptr_t) *arg < 0x800000000000) {
            // address is on stack, lookup the destination
            void **ptr = (void**) *arg;
            auto it = driver_allocation_to_var_name.find(*ptr);
            if (it != driver_allocation_to_var_name.end()) {
                params += "&" + it->second + ",";
                return;
            }
        }

        std::stringstream ss;
        ss << *arg;

        params += "TODO_input(" + ss.str() + "),";
    }
    void addArg(const void **arg, int param_index) {
        if (*arg == nullptr) {
            params += "nullptr,";
            return;
        }

        auto it = driver_allocation_to_var_name.find(const_cast<void*>(*arg));
        if (it != driver_allocation_to_var_name.end()) {
            params += it->second + ",";
            return;
        }

        if ((uintptr_t) *arg > 0x700000000000 && (uintptr_t) *arg < 0x800000000000) {
            // address is on stack, lookup the destination
            void **ptr = (void**) *const_cast<void**>(arg);
            auto it = driver_allocation_to_var_name.find(*ptr);
            if (it != driver_allocation_to_var_name.end()) {
                params += "&" + it->second + ",";
                return;
            }
        }

        std::stringstream ss;
        ss << *arg;

        // if nothing is found, this can be a pointer to host memory
        // we can try to figure out size and read it, but this would
        // make the generate code huge
        params += "TODO_host_ptr(" + ss.str() + "),";
    }
    void addArg(void ***arg, int param_index) {
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
    void addArg(std::integral auto *arg, int param_index) {
        auto value = *arg;
        lastSize = value;
        params += std::to_string(value) + ",";
    }
    void addArg(std::integral auto **arg, int param_index) {
        if (*arg == nullptr) {
            params += "nullptr,";
            return;
        }

        lastSize = **arg;

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
    void addArg(Handle auto *arg, int param_index) {
        if (*arg == nullptr) {
            params += "nullptr,";
            return;
        }

        if (arg_to_var_name.find((void*)(*arg)) == arg_to_var_name.end()) {
            //std::cerr << *arg << std::endl;
            params += "TODO_handle,";
            return;
        }

        params += arg_to_var_name[(void*)(*arg)] + ",";
    }
    void addArg(Handle auto **arg, int param_index) {
        if (*arg == nullptr) {
            params += "nullptr,";
            return;
        }

        auto [type_name, description] = zeApi.get_typename_and_description(fname, param_index);

        if (description.find("range(") != std::string::npos) {
            // assert(param_index > 0);
            auto varName = nextVarName("arr_" + type_name_to_var_name(type_name));
            init += "\n" + trim_ptr(type_name) + " " + varName + "[" + std::to_string(lastSize) + "];";
           // std::cerr << "lastSize: " << lastSize << std::endl;
            for (int i = 0; i < lastSize; i++) {
                //std::cerr << i << " " << (*arg)[i] << std::endl;
                arg_to_var_name[(void*)(*arg)[i]] = varName + "[" + std::to_string(i) + "]";
                init += "\n" + varName + "[" + std::to_string(i) + "] = &" + arg_to_var_name[(void*)(*arg)[i]] + ";";
            }
            params += varName + ",";
            arg_to_var_name[(void*)(*arg)] = varName;
            return;
        }

        if (arg_to_var_name.find((void*)(**arg)) != arg_to_var_name.end()) {
            // handle shouldn't be know, it should be either array (already handled) or out param
            params += "TODO,";
            return;
        }
        // otherwise it's output parameter
        auto varName = nextVarName(type_name_to_var_name(type_name));
        init += "\n" + trim_ptr(type_name) + " " + varName + "{};";
        arg_to_var_name[(void*)(**arg)] = varName;
        params += "&" + varName + ",";
    }
    void addArg(Properties auto **arg, int param_index) {
        params += "TODOproperties,";
        skip = true;
    }
    void addArg(Descriptor auto **arg, int param_index) {
        auto [var, in] = initDescriptor(**arg);
        init += in;
        params += "&" + var + ",";
    }

    std::string init;
    std::string params;
    std::string fname;
    size_t lastSize = 0;
    bool skip = false;

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

Args make_args(std::string fname, auto params) {
    auto tuple = make_tuple(params);

    Args args(fname);

    int param_index = 0;
    std::apply([&](auto... a) {
        (args.addArg(a, param_index++), ...);
    }, tuple);

    return args;
}

std::string desc_member_to_string(auto member) {
    return "TODO";
}

template <Descriptor D>
std::string desc_member_to_string(D member) {
    return getZeStructureType(member);
}

std::string desc_member_to_string(Stringifiable auto member) {
    return std::to_string(member);
}

std::string desc_member_to_string(Stringifiable auto *member) {
    return std::to_string(*member);
}

std::string desc_member_to_string(const char* member) {
    return "\"" + std::string(member) + "\"";
}

// std::stringstream os;

void ze_cb_invoke(std::string_view functionName, Args&& args, ze_result_t result) {
    std::ignore = result;

    if (args.skip) {
        return;
    }

    std::cout << args.init << "\n";
    std::cout << "ZE_CALL(" << functionName << "(";
    args.params.pop_back(); // remove trailing comma
    std::cout << args.params;
    std::cout << "));\n";
}

