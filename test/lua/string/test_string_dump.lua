function test_string_dump()
    -- Case 1: 无闭包/普通表达式的 string.dump -> load -> 运行校验返回值
    local code1 = "123 + 456"
    local fn1 = load(code1)
    if fn1 == nil then return 10 end

    local dumped1 = string.dump(fn1)
    if string.sub(dumped1, 1, 4) ~= "\027Lua" then return 20 end

    local fn1_restore = load(dumped1)
    if fn1_restore == nil then return 30 end

    local res1 = fn1_restore()
    if res1 ~= 579 then return 40 end

    -- Case 2: 包含局部作用域与嵌套子函数的闭包的 string.dump -> load -> 运行校验返回值
    local code2 = "local x = 100\nlocal function add_up(y) return x + y end\nreturn add_up(50)"
    local fn2 = load(code2)
    if fn2 == nil then return 50 end

    local dumped2 = string.dump(fn2)
    if string.sub(dumped2, 1, 4) ~= "\027Lua" then return 60 end

    local fn2_restore = load(dumped2)
    if fn2_restore == nil then return 70 end

    local res2 = fn2_restore()
    if res2 ~= 150 then return 80 end

    return 700
end
