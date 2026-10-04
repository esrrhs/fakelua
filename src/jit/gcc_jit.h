#pragma once

#include "compile/compile_common.h"
#include "fakelua.h"
#include <functional>
#include <memory>
#include <string>

namespace fakelua {

class State;

class GccJitter {
public:
    explicit GccJitter(State *s);

    void Compile(const ParseResult &pr, const GenResult &gr, const CompileConfig &cfg);

private:
    // 产物加载完成后的统一收尾：注入 State → 注册函数 → 调用 init。
    // 缓存命中与正常编译两条路径共用，避免行为漂移。
    void RegisterAndInit(const ParseResult &pr, const GenResult &gr,
                         const std::shared_ptr<JITHandle> &handle,
                         const std::function<void *(const std::string &)> &dlsym_lambda,
                         const std::string &artifact_path);

    State *s_;
};

}// namespace fakelua
