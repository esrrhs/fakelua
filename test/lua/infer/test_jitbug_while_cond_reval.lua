-- P1-9 回归：while 条件必须每轮重新求值。
-- 原生 C while 快速路径只能嵌入「不输出语句」的纯表达式条件；#t、函数调用、
-- 表索引、整数整除/取模等经 CompileNumericExp 会编译成 while 之外只求值一次的
-- 「语句 + 临时变量」，造成条件读陈旧值（死循环或越界）。这些条件必须回退到
-- while(1){ 每轮重新 CompileExp 条件 } 的通用路径。
-- 预期值以 PUC-Rio Lua 5.4/5.5 实跑结果为准。

-- #t 每轮重新求值：陈旧绑定会死循环（100 次保护返回 -1），正确则删空返回 10。
function test_len_in_cond()
    local t = {}
    for i = 1, 10 do t[i] = i end
    local removed = 0
    while #t > 0 do
        if removed > 100 then return -1 end
        table.remove(t, #t)
        removed = removed + 1
    end
    return removed
end

-- 条件里的函数调用每轮重新执行。
function test_call_in_cond()
    local n = 3
    local function dec()
        n = n - 1
        return n
    end
    local hits = 0
    while dec() > 0 do
        hits = hits + 1
    end
    return hits
end

-- 条件里的表索引每轮重新读取。
function test_index_in_cond()
    local t = { x = 3 }
    local hits = 0
    while t.x > 0 do
        t.x = t.x - 1
        hits = hits + 1
    end
    return hits
end

-- 条件里的整数取模 / 整除每轮重新计算。
function test_arith_in_cond()
    local v = 10
    local hits = 0
    while v % 3 ~= 0 do
        v = v - 1
        hits = hits + 1
    end
    local v2 = 10
    local hits2 = 0
    while v2 // 4 > 1 do
        v2 = v2 - 1
        hits2 = hits2 + 1
    end
    return v * 1000 + hits * 100 + v2 * 10 + hits2
end

-- not 包住的不纯比较同样必须回退。
function test_not_len_in_cond()
    local t = { 1, 2, 3 }
    local hits = 0
    while not (#t == 0) do
        if hits > 10 then return -1 end
        table.remove(t, 1)
        hits = hits + 1
    end
    return hits
end

-- 纯简单变量条件仍走原生 while 快速路径（结果也必须正确）。
function test_pure_cond()
    local x = 0
    local hits = 0
    while x < 10 do
        x = x + 2
        hits = hits + 1
    end
    return hits * 100 + x
end

function test()
    if test_len_in_cond() ~= 10 then return 1 end
    if test_call_in_cond() ~= 2 then return 2 end
    if test_index_in_cond() ~= 3 then return 3 end
    if test_arith_in_cond() ~= 9173 then return 4 end
    if test_not_len_in_cond() ~= 3 then return 5 end
    if test_pure_cond() ~= 510 then return 6 end
    return 0
end
