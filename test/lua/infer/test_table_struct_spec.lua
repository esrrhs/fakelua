-- table struct specialization 测试
function test_struct_spec()
    local t = { x = 10, y = 20 }
    t.x = 99
    return t.x + t.y
end

-- 单字段 table
function test_single_field()
    local t = { val = 42 }
    return t.val
end

-- 写入后读取
function test_write_then_read()
    local t = { a = 1, b = 2, c = 3 }
    t.a = 10
    t.b = 20
    t.c = 30
    return t.a + t.b + t.c
end

-- 遍历特化 table（pairs 走 hash，初始化时双写保证可用）
function test_pairs_iter()
    local t = { x = 10, y = 20 }
    local sum = 0
    for k, v in pairs(t) do
        sum = sum + v
    end
    return sum
end

-- 动态 key 访问（走 hash fallback）
function test_dynamic_key(k, v)
    local t = { x = 10 }
    t[k] = v
    return t.x
end

-- 特化 table 传参后动态访问
function test_pass_to_func(k)
    local t = { a = 1, b = 2 }
    return t[k]
end

-- 多个特化 table
function test_multi_tables()
    local t1 = { x = 10 }
    local t2 = { y = 20 }
    return t1.x + t2.y
end

-- 嵌套 table（外层特化，内层不特化）
function test_nested()
    local t = { inner = { v = 5 } }
    return t.inner.v
end

-- 三字段求和
function test_three_fields()
    local t = { a = 100, b = 200, c = 300 }
    return t.a + t.b + t.c
end

-- 写入非 spec 字段（fallback 到 hash）
function test_dynamic_write_and_read()
    local t = { x = 1 }
    t["y"] = 2
    return t.x + t.y
end
