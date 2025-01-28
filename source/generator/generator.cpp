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

#define ZE_CALL(status, ex) \
do {\
auto status_ = status;\
if (status_ != ex) { \
    std::cerr << "Error: " << status_ << "\n"; \
    std::exit(1); \
}\
} while(0);

inline std::shared_ptr<_zel_tracer_handle_t>
enableTracing(zel_core_callbacks_t &&prologueCallbacks,
              zel_core_callbacks_t &&epilogueCallbacks) {
  ZE_CALL(zeInit(ZE_INIT_FLAG_GPU_ONLY), ZE_RESULT_SUCCESS);

  zel_tracer_desc_t tracer_desc = {ZEL_STRUCTURE_TYPE_TRACER_EXP_DESC, nullptr,
                                   nullptr};
  zel_tracer_handle_t tracer = nullptr;
  ZE_CALL(zelTracerCreate(&tracer_desc, &tracer), ZE_RESULT_SUCCESS);

  ZE_CALL(zelTracerSetPrologues(tracer, &prologueCallbacks),
            ZE_RESULT_SUCCESS);
  ZE_CALL(zelTracerSetEpilogues(tracer, &epilogueCallbacks),
            ZE_RESULT_SUCCESS);
  ZE_CALL(zelTracerSetEnabled(tracer, true), ZE_RESULT_SUCCESS);

  return std::shared_ptr<_zel_tracer_handle_t>(
      tracer, [](zel_tracer_handle_t tracer) { zelTracerDestroy(tracer); });
}

struct InArg {
};

struct OutArg {
};

struct InOutArg {
};

static int next_var_id = 0;
std::unordered_map<void*, std::string> arg_to_var_name;

std::string nextVarName(std::string_view prefix) {
    return std::string(prefix) + std::to_string(next_var_id++);
}

template<typename T, typename ... U>
concept AnyOf = (std::same_as<T, U> || ...);

template <typename T> concept Handle = AnyOf<T,
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

template <typename T> concept Descriptor = requires(T t) {
    { t.stype } -> std::convertible_to<const ze_structure_type_t&>;
};

struct Args {
    void addArg(std::string name, auto arg, auto type) {
        // need manual fixup
        params += "TODO_" + name + ",";
    }

    void addArg(std::string name, auto arg, OutArg) {
        // by default, ignore out args, we'll specialie this
        // for types we really care about
    }

    void addArg(std::string name, std::integral auto *arg, InArg) {
        auto value = *arg;
        params += std::to_string(value) + ",";
    }

    void addArg(std::string name, Handle auto *arg, InArg) {
        params += "device_or_command_list,";
    }

    void addArg(std::string name, Descriptor auto **arg, InArg, std::vector<std::string> initValues) {
        auto varName = nextVarName(name)
        init += "auto " + varName + "{};\n";
        for (auto &v : initValues) {
            init += varName + "." + v;
        }
    }

    std::string init;
    std::string params;
};

// std::stringstream os;


void ze_cb_invoke(std::string_view functionName, Args&& args, ze_result_t result) {
    std::ignore = result;

    std::cout << args.init << "\n";
    std::cout << "ZE_CALL(" << functionName << "(";
    std::cout << args.params;
    std::cout << "))\n";
}

struct arg {
 // pointer to member + value

 template <auto member_ptr> 
 arg(auto value) {
    store = [this, value] {
        *member_ptr = value;
    };
 }

 std::function<void> store;
};

Args make_args(ze_mem_alloc_device_params_t params) { 
    Args args;
    args.addArg("hContext", params.phContext, InArg{});
    args.addArg("device_desc", params.pdevice_desc ,InArg{});
    args.addArg("size", params.psize, InArg{});
    args.addArg("hDevice", params.phDevice, InArg{});
    return args;
 }

void __attribute__((constructor)) Load() {
  zel_core_callbacks_t epilogueCallbacks{};
  epilogueCallbacks.Mem.pfnAllocDeviceCb = [](ze_mem_alloc_device_params_t* params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeMemAllocDevice", make_args(*params), result);
    };
  enableTracing({}, std::move(epilogueCallbacks));
}

void __attribute__((destructor)) Unload() {
}

