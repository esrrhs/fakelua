-- 回归：顶层 Lua 函数若与 C 入口名（main）或 libc/头文件符号（sin 等）同名，
-- 生成的 C 符号必须统一加 flua_fn_ 前缀。否则：
--   * 名为 main 的函数：Clang 报 first parameter of 'main' must be of type 'int'；
--   * 名为 sin 的函数：TCC 报 incompatible types for redefinition of 'sin'。
-- Lua 侧仍以原名调用。

function main(x)
    return x + 1
end

function sin(x)
    return x * 2
end

function test()
    if main(41) ~= 42 then
        return 1
    end
    if sin(21) ~= 42 then
        return 2
    end
    return 0
end
