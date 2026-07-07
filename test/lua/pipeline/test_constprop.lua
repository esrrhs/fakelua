-- 常量传播: local key = "b"; a[key] 应推导为 a.b
function test()
    local a = {b = 1, c = 2}
    local key = "b"
    local v = a[key]
    return v
end
