function test_string_dump()
    -- Case 1: 表达式 load -> dump -> load -> 运行校验返回值
    local code1 = "123 + 456"
    local fn1 = load(code1)
    if fn1 == nil then return 10 end

    local dumped1 = string.dump(fn1)
    if string.sub(dumped1, 1, 4) ~= "\027Lua" then return 20 end

    local fn1_restore = load(dumped1)
    if fn1_restore == nil then return 30 end

    local res1 = fn1_restore()
    if res1 ~= 579 then return 40 end

    -- Case 2: 作用域子函数 load -> dump -> load -> 运行校验返回值
    local code2 = "local x = 100\nlocal function add_up(y) return x + y end\nreturn add_up(50)"
    local fn2 = load(code2)
    if fn2 == nil then return 50 end

    local dumped2 = string.dump(fn2)
    if string.sub(dumped2, 1, 4) ~= "\027Lua" then return 60 end

    local fn2_restore = load(dumped2)
    if fn2_restore == nil then return 70 end

    local res2 = fn2_restore()
    if res2 ~= 150 then return 80 end

    -- Case 3: 静态闭包 (直接声明，无先 load)，校验 string.dump 魔数导出与 load 的安全性
    local static_closure = function(a, b)
        return a * 10 + b
    end

    local dumped3 = string.dump(static_closure)
    if string.sub(dumped3, 1, 4) ~= "\027Lua" then return 90 end

    local static_restore = load(dumped3)
    if static_restore == nil then return 100 end

    -- Case 4: 带 Upvalue 的静态闭包，校验 string.dump 魔数导出与 load 的安全性
    local base_val = 200
    local static_closure_with_upvalue = function(a)
        return base_val + a
    end

    local dumped4 = string.dump(static_closure_with_upvalue)
    if string.sub(dumped4, 1, 4) ~= "\027Lua" then return 120 end

    local static_upval_restore = load(dumped4)
    if static_upval_restore == nil then return 130 end

    return 700
end
