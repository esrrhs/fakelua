-- INT64_MIN // -1 与 INT64_MIN % -1 在 C 里是 UB（x86 上会 SIGFPE）。
-- 对齐 Lua 5.4：idiv 用无符号取负（结果仍是 mininteger），mod 为 0。
-- 动态路径 -mininteger 应变为 float 2^63（与 math.abs 一致）。

function test_math_int64_min_arith()
    local mini = math.mininteger

    -- 表取值是 T_DYNAMIC，走 OpFloorDiv / OpMod / OpUnaryMinus
    local t = {mini, -1}
    if t[1] // t[2] ~= mini then return 1 end
    if t[1] % t[2] ~= 0 then return 2 end
    local unm = -t[1]
    if math.type(unm) ~= "float" then return 3 end
    if unm ~= 9223372036854775808.0 then return 4 end

    -- 本地整数走 FlFloorDivInt / FlModInt
    local a = math.mininteger
    local b = -1
    if a // b ~= mini then return 5 end
    if a % b ~= 0 then return 6 end

    -- 普通向下取整回归，避免修特殊路径时把语义改坏
    if 7 // 2 ~= 3 then return 7 end
    if -7 // 2 ~= -4 then return 8 end
    if 7 % 2 ~= 1 then return 9 end
    if -7 % 2 ~= 1 then return 10 end
    if -7 % -2 ~= -1 then return 11 end

    return 100
end
