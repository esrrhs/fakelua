-- 回归：尾参数展开路径曾把「语句宏」(OpAdd 展开成 do{}while(0)、math.floor 的
-- if/else 类型分支) 直接拼接进表达式位置，生成非法 C：expression expected before 'do'。
-- 触发条件：最后一个实参为需要展开的函数调用，且该调用（或其嵌套子表达式）含
-- 动态类型操作数的算术（os.time() 等原生调用结果推导为 T_DYNAMIC）。

local function consume(x, y)
    if x ~= 1 then
        return -1
    end
    return y
end

-- 多返回值 local 函数：max_returns=2，调用点走尾展开分支
local function two(x)
    return x, x + 100
end

-- generic for 用迭代器
local function it(tt, idx)
    idx = idx + 1
    local v = tt[idx]
    if v then
        return idx, v
    end
end

function test()
    local a = os.time()              -- T_DYNAMIC，使 a + 1 走慢路径语句宏
    -- 变体 A：尾实参为原生函数 math.floor，嵌套动态算术 a + 1
    local r1 = consume(1, math.floor(a + 1))
    if type(r1) ~= "number" then
        return 1
    end
    -- 变体 B：尾实参为多返回值 local 函数 two，实参 a + 1 含动态算术
    local r2 = consume(1, two(a + 1))
    if r2 ~= a + 1 then
        return 2
    end
    -- 变体 C：generic for 超过 3 个表达式时，第 4 个（丢弃）表达式含同类语句宏
    local t = {10, 20, 30}
    local sum = 0
    local cnt = 0
    for idx, v in it, t, 0, math.floor(a + 1) do
        sum = sum + v
        cnt = cnt + 1
    end
    if cnt ~= 3 then
        return 3
    end
    return sum
end
