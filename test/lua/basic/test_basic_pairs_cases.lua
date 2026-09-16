package "BasicPairsCases"

-- 测试 pairs 基本迭代
function test_pairs_basic()
    local t = { a = 1, b = 2, c = 3 }
    local count = 0
    for k, v in pairs(t) do
        count = count + 1
    end
    if count ~= 3 then return 0 end
    return 1
end

-- 测试 ipairs 基本迭代
function test_ipairs_basic()
    local t = { 10, 20, 30 }
    local count = 0
    local sum = 0
    for i, v in ipairs(t) do
        count = count + 1
        sum = sum + v
    end
    if count ~= 3 then return 0 end
    if sum ~= 60 then return 0 end
    return 1
end

-- 测试 ipairs 非连续（遇到第一个 nil 即停止）
function test_ipairs_non_continuous()
    local t = { 10, nil, 30 }
    local count = 0
    local sum = 0
    for i, v in ipairs(t) do
        count = count + 1
        sum = sum + v
    end
    if count ~= 1 then return 0 end
    if sum ~= 10 then return 0 end

    local t2 = { [1] = 10, [2] = 20, [4] = 40 }
    count = 0
    sum = 0
    for i, v in ipairs(t2) do
        count = count + 1
        sum = sum + v
    end
    if count ~= 2 then return 0 end
    if sum ~= 30 then return 0 end
    return 1
end

-- 测试 pairs 空表
function test_pairs_empty()
    local t = {}
    local count = 0
    for k, v in pairs(t) do
        count = count + 1
    end
    if count ~= 0 then return 0 end
    return 1
end

-- 局部 ipairs 必须走用户函数，不能被 pairs/ipairs 快路径吞掉
function test_ipairs_local_shadow()
    local function ipairs(t)
        local i = 0
        return function()
            i = i + 1
            if i > 2 then return nil end
            return i, t[i] * 10
        end
    end
    local t = { 1, 2, 3 }
    local n = 0
    local sum = 0
    for i, v in ipairs(t) do
        n = n + 1
        sum = sum + v
    end
    if n ~= 2 then return 0 end
    if sum ~= 30 then return 0 end
    return 1
end
