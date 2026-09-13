-- 全局常量 table 特化测试
local const_t = { x = 10, y = 20 }

function test_global_spec()
    return const_t.x + const_t.y
end
