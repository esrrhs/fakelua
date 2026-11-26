#pragma once

#include "fakelua.h"
#include <filesystem>
#include <functional>
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
    // Like Restore, but on POSIX may return the stable cached image path directly
    // when this process has not yet loaded that cache path (avoids per-copy dynamic
    // loader / macOS syspolicyd overhead). If direct loading is not eligible, copies
    // the image to destination (invoking ensure_destination first) and sets load_path.
    bool RestoreForLoad(const std::string &destination, std::string &load_path, bool &is_direct, const std::function<void()> &ensure_destination = {});
    void Publish(const std::string &binary, const std::string &dependencies, const std::string &source);
    // On POSIX, after Publish(), attempt to claim the published cache image path for
    // direct dlopen once per process so the OS code-signing trust cache is warmed for
    // subsequent launches. Returns empty string if unavailable or already claimed.
    std::string TryClaimPublishedImage();
    void Invalidate();
    void Release();

private:
    // Process-wide claim of a stable cache image path for direct POSIX dlopen.
    // The first claim in this process wins; later loads of the same path copy the image.
    static bool TryClaimDirectImagePath(const std::filesystem::path &image_path);

    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}// namespace fakelua
