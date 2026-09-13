package "BasicSelectCases"

-- 测试 select 正数索引
function test_select_positive()
    local a, b = select(2, "x", "y", "z")
    if a ~= "y" then return 0 end
    if b ~= "z" then return 0 end
    return 1
end

-- 测试 select 负数索引
function test_select_negative()
    local a = select(-1, "x", "y", "z")
    if a ~= "z" then return 0 end
    return 1
end

-- 测试 select # 计数
function test_select_count()
    local n = select("#", "a", "b", "c")
    if n ~= 3 then return 0 end
    return 1
end

-- 测试 select 越界
function test_select_out_of_range()
    local r = select(10, "a", "b")
    if r ~= nil then return 0 end
    return 1
end
