/*
 *
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef UR_UTIL_H
#define UR_UTIL_H 1

#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string.h>
#include <string>
#include <vector>

/* for compatibility with non-clang compilers */
#if defined(__has_feature)
#define CLANG_HAS_FEATURE(x) __has_feature(x)
#else
#define CLANG_HAS_FEATURE(x) 0
#endif

/* define for running with address sanitizer */
#if CLANG_HAS_FEATURE(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
#define SANITIZER_ADDRESS
#endif

/* define for running with memory sanitizer */
#if CLANG_HAS_FEATURE(memory_sanitizer)
#define SANITIZER_MEMORY
#endif

/* define for running with any sanitizer runtime */
#if defined(SANITIZER_MEMORY) || defined(SANITIZER_ADDRESS)
#define SANITIZER_ANY
#endif

///////////////////////////////////////////////////////////////////////////////
#if defined(_WIN32)
#include <Windows.h>
#define MAKE_LIBRARY_NAME(NAME, VERSION) NAME ".dll"
#define MAKE_LAYER_NAME(NAME) NAME ".dll"
#define LOAD_DRIVER_LIBRARY(NAME) LoadLibraryExA(NAME, nullptr, 0)
#define FREE_DRIVER_LIBRARY(LIB) \
    if (LIB)                     \
    FreeLibrary(LIB)
#define GET_FUNCTION_PTR(LIB, FUNC_NAME) GetProcAddress(LIB, FUNC_NAME)
#define string_copy_s strncpy_s
#else
#include <dlfcn.h>
#define HMODULE void *
#define MAKE_LIBRARY_NAME(NAME, VERSION) "lib" NAME ".so." VERSION
#define MAKE_LAYER_NAME(NAME) "lib" NAME ".so." L0_VALIDATION_LAYER_SUPPORTED_VERSION
#if defined(SANITIZER_ANY)
#define LOAD_DRIVER_LIBRARY(NAME) dlopen(NAME, RTLD_LAZY | RTLD_LOCAL)
#else
#define LOAD_DRIVER_LIBRARY(NAME) dlopen(NAME, RTLD_LAZY | RTLD_LOCAL | RTLD_DEEPBIND)
#endif
#define FREE_DRIVER_LIBRARY(LIB) \
    if (LIB)                     \
    dlclose(LIB)
#define GET_FUNCTION_PTR(LIB, FUNC_NAME) dlsym(LIB, FUNC_NAME)
#define string_copy_s strncpy
#endif

inline std::string create_library_path(const char *name, const char *path) {
    std::string library_path;
    if (path && (strcmp("", path) != 0)) {
        library_path.assign(path);
#ifdef _WIN32
        library_path.append("\\");
#else
        library_path.append("/");
#endif
        library_path.append(name);
    } else {
        library_path.assign(name);
    }
    return library_path;
}

//////////////////////////////////////////////////////////////////////////
#if !defined(_WIN32) && (__GNUC__ >= 4)
#define __urdlllocal __attribute__((visibility("hidden")))
#else
#define __urdlllocal
#endif

///////////////////////////////////////////////////////////////////////////////
inline std::optional<std::string> ur_getenv(const char *name) {
#if defined(_WIN32)
    constexpr int buffer_size = 1024;
    char buffer[buffer_size];
    auto rc = GetEnvironmentVariable(name, buffer, buffer_size);
    if (0 != rc && rc < buffer_size) {
        return std::string(buffer);
    } else if (rc >= buffer_size) {
        std::stringstream ex_ss;
        ex_ss << "Environment variable " << name << " value too long!"
              << " Maximum length is " << buffer_size - 1 << " characters.";
        throw std::invalid_argument(ex_ss.str());
    }
    return std::nullopt;
#else
    const char *tmp_env = getenv(name);
    if (tmp_env != nullptr) {
        return std::string(tmp_env);
    } else {
        return std::nullopt;
    }
#endif
}

inline bool getenv_tobool(const char *name) {
    auto env = ur_getenv(name);
    return env.has_value();
}

static void throw_wrong_format_vec(const char *env_var_name) {
    std::stringstream ex_ss;
    ex_ss << "Wrong format of the " << env_var_name << " environment variable!"
          << " Proper format is: ENV_VAR=\"value_1,value_2,value_3\"";
    throw std::invalid_argument(ex_ss.str());
}

static void throw_wrong_format_map(const char *env_var_name) {
    std::stringstream ex_ss;
    ex_ss << "Wrong format of the " << env_var_name << " environment variable!"
          << " Proper format is: ENV_VAR=\"param_1:value_1,value_2;param_2:value_1";
    throw std::invalid_argument(ex_ss.str());
}

/// @brief Get a vector of values from an environment variable \p env_var_name
///        A comma is a delimiter for extracting values from env var string.
///        Colons and semicolons are not allowed to align with the similar
///        getenv_to_map() util function and avoid confusion.
///        A vector with a single value is allowed.
///        Env var must consist of strings separated by commas, ie.:
///        ENV_VAR=1,4K,2M
/// @param env_var_name name of an environment variable to be parsed
/// @return std::optional with a possible vector of strings containing parsed values
///         and std::nullopt when the environment variable is not set or is empty
/// @throws std::invalid_argument() when the parsed environment variable has wrong format
inline std::optional<std::vector<std::string>> getenv_to_vec(const char *env_var_name) {
    char values_delim = ',';

    auto env_var = ur_getenv(env_var_name);
    if (!env_var.has_value()) {
        return std::nullopt;
    }

    if (env_var->find(':') == std::string::npos && env_var->find(';') == std::string::npos) {
        std::stringstream values_ss(*env_var);
        std::string value;
        std::vector<std::string> values_vec;
        while (std::getline(values_ss, value, values_delim)) {
            if (value.empty()) {
                throw_wrong_format_vec(env_var_name);
            }
            values_vec.push_back(value);
        }
        return values_vec;
    } else {
        throw_wrong_format_vec(env_var_name);
    }
    return std::nullopt;
}

using EnvVarMap = std::map<std::string, std::vector<std::string>>;

/// @brief Get a map of parameters and their values from an environment variable
///        \p env_var_name
///        Semicolon is a delimiter for extracting key-values pairs from
///        an env var string. Colon is a delimiter for splitting key-values pairs
///        into keys and their values. Comma is a delimiter for values.
///        All special characters in parameter and value strings are allowed except
///        the delimiters.
///        Env vars without parameter names are not allowed, use the getenv_to_vec()
///        util function instead.
///        Keys in a map are parsed parameters and values are vectors of strings
///        containing parameters' values, ie.:
///        ENV_VAR="param_1:value_1,value_2;param_2:value_1"
///        result map:
///             map[param_1] = [value_1, value_2]
///             map[param_2] = [value_1]
/// @param env_var_name name of an environment variable to be parsed
/// @return std::optional with a possible map with parsed parameters as keys and
///         vectors of strings containing parsed values as keys.
///         Otherwise, optional is set to std::nullopt when the environment variable
///         is not set or is empty.
/// @throws std::invalid_argument() when the parsed environment variable has wrong format
inline std::optional<EnvVarMap> getenv_to_map(const char *env_var_name) {
    char main_delim = ';';
    char key_value_delim = ':';
    char values_delim = ',';
    EnvVarMap map;

    auto env_var = ur_getenv(env_var_name);
    if (!env_var.has_value()) {
        return std::nullopt;
    }

    std::stringstream ss(*env_var);
    std::string key_value;
    while (std::getline(ss, key_value, main_delim)) {
        std::string key;
        std::string values;
        std::stringstream kv_ss(key_value);

        if (key_value.find(':') == std::string::npos) {
            throw_wrong_format_map(env_var_name);
        }

        std::getline(kv_ss, key, key_value_delim);
        std::getline(kv_ss, values);
        if (key.empty() || values.empty() || values.find(':') != std::string::npos || map.find(key) != map.end()) {
            throw_wrong_format_map(env_var_name);
        }

        std::vector<std::string> values_vec;
        std::stringstream values_ss(values);
        std::string value;
        while (std::getline(values_ss, value, values_delim)) {
            if (value.empty()) {
                throw_wrong_format_map(env_var_name);
            }
            values_vec.push_back(value);
        }
        if (values_vec.empty()) {
            throw_wrong_format_map(env_var_name);
        }
        map[key] = values_vec;
    }
    return map;
}

inline std::size_t combine_hashes(std::size_t seed) {
    return seed;
}

template <typename T, typename... Args>
inline std::size_t combine_hashes(std::size_t seed, const T &v, Args... args) {
    return combine_hashes(seed ^ std::hash<T>{}(v), args...);
}

#endif /* UR_UTIL_H */
