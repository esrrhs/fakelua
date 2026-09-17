-- 函数 C 名 sanitization 后与另一函数 / 文件级 local 撞名时必须去重，否则 GCC 编译失败。

function int()
    return 1
end

function flua_id_int()
    return 2
end

function test_c_func_name_collision()
    return int() + flua_id_int()
end

-- class 是 C++ 关键字 → flua_id_class，与函数 flua_id_class 撞 C 符号。
local class = 7

function flua_id_class()
    return 100
end

function test_c_global_vs_func_collision()
    return class + flua_id_class()
end
