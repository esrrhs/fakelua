#pragma once

#include <string>
#include <list>
#include <vector>
#include <map>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <typeinfo>
#include <time.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>
#include <fstream>
#include <filesystem>
#include <string_view>
#include <experimental/source_location>
#include <sstream>

const std::string LOG_DEBUG = "DEBUG";
const std::string LOG_ERROR = "ERROR";

template<typename FormatString, typename... Args>
inline void log(const std::experimental::source_location &location, const std::string &level, const FormatString &fmt,
                Args &&...args) {
    char buf[256] = {0};
    snprintf(buf, sizeof(buf), fmt, std::forward<Args>(args)...);
    std::stringstream ss;
    ss << "[" << level << "]" << location.function_name() << "("
       << std::filesystem::path(location.file_name()).filename().string() << ":"
       << location.line() << "): " << buf << std::endl;
    std::cout << (ss.str());
    std::ofstream ofs;
    ofs.open(level + ".log", std::ios::out | std::ios::app);
    if (!ofs.is_open()) {
        std::cout << "Error opening " << level << ".log for output" << std::endl;
        return;
    }
    ofs << (ss.str());
    ofs.close();
}

template<typename FormatString, typename... Args>
inline void debug(const std::experimental::source_location &location, const FormatString &fmt, Args &&...args) {
    log(location, LOG_DEBUG, fmt, std::forward<Args>(args)...);
}

template<typename FormatString, typename... Args>
inline void error(const std::experimental::source_location &location, const FormatString &fmt, Args &&...args) {
    log(location, LOG_ERROR, fmt, std::forward<Args>(args)...);
}

#ifndef NDEBUG
#define DEBUG(...) debug(std::experimental::source_location::current(), __VA_ARGS__)
#define ERR(...) error(std::experimental::source_location::current(), __VA_ARGS__)
#else
#define DEBUG
#define ERR
#endif
