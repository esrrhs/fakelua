-- 多次写入后读取
function test_write_multi()
    local t = { a = 1, b = 2, c = 3 }
    t.a = 10
    t.b = 20
    t.c = 30
    return t.a + t.b + t.c
end
