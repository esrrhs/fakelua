#pragma once

#include "fakelua.h"
#include <filesystem>
#include <memory>
#include <string>

namespace fakelua {
// A best-effort, private, content-addressed compiler artifact cache. Never load
// the cache inode itself: generated code contains State-local mutable statics.
class GccCache {
public:
    GccCache(const StateGCCConfig &config, const std::string &compiler, const std::string &code, bool debug);
    ~GccCache();
    bool Restore(const std::string &destination);
    void Publish(const std::string &binary, const std::string &dependencies, const std::string &source);
    void Invalidate();
    void Release();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}// namespace fakelua
