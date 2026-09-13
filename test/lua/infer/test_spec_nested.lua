-- 嵌套 table（外层特化，内层不特化）
function test_nested()
    local t = { inner = { v = 5 } }
    return t.inner.v
end
