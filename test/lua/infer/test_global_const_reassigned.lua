-- 文件级数值变量声明后有多个再赋值点（且全部是编译期常量）。
-- 曾被 JIT 误标为 static const，导致生成的 C 代码编译失败：
--   error: cannot assign to variable 'next_bot_id' with const-qualified type
-- 修复后：有再赋值点的变量发射为非 const 的 static int64_t，赋值语义与 Lua 一致。
local next_bot_id = 0

function init()
    next_bot_id = 800000
    return next_bot_id
end

function add()
    next_bot_id = next_bot_id + 1
    return next_bot_id
end

function get()
    return next_bot_id
end
