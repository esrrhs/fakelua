-- Lua 5.4 table comprehensive tests

function test_table_basic()
    local t = {10, 20, 30, 40, 50}
    if #t ~= 5 then return 0 end
    if t[1] ~= 10 then return 0 end
    if t[5] ~= 50 then return 0 end
    return 1
end

function test_table_string_keys()
    local t = {name = "Alice", age = 30, city = "NYC"}
    if t.name ~= "Alice" then return 0 end
    if t.age ~= 30 then return 0 end
    if t.city ~= "NYC" then return 0 end
    return 1
end

function test_table_buildup()
    local t = {}
    for i = 1, 100 do
        t[i] = i * i
    end
    if #t ~= 100 then return 0 end
    if t[1] ~= 1 then return 0 end
    if t[10] ~= 100 then return 0 end
    if t[100] ~= 10000 then return 0 end
    return 1
end

function test_table_mixed()
    local t = {}
    t[1] = "one"
    t[2] = "two"
    t["key"] = "value"
    t[true] = "yes"
    if t[1] ~= "one" then return 0 end
    if t["key"] ~= "value" then return 0 end
    if t[true] ~= "yes" then return 0 end
    return 1
end

function test_table_length()
    if #{} ~= 0 then return 0 end
    if #{1} ~= 1 then return 0 end
    if #{1, 2, 3} ~= 3 then return 0 end
    return 1
end

function test_table_sort()
    local t = {5, 3, 8, 1, 9, 2, 7, 4, 6}
    local n = #t
    for i = 1, n do
        for j = 1, n - i do
            if t[j] > t[j + 1] then
                local tmp = t[j]
                t[j] = t[j + 1]
                t[j + 1] = tmp
            end
        end
    end
    for i = 1, n - 1 do
        if t[i] > t[i + 1] then return 0 end
    end
    return 1
end

function test_table_matrix()
    local m = {}
    for i = 1, 3 do
        m[i] = {}
        for j = 1, 3 do
            m[i][j] = i * 3 + j
        end
    end
    if m[1][1] ~= 4 then return 0 end
    if m[2][3] ~= 9 then return 0 end
    if m[3][2] ~= 11 then return 0 end
    return 1
end

function test_table_sieve()
    local n = 50
    local is_prime = {}
    for i = 2, n do is_prime[i] = true end
    local i = 2
    while i * i <= n do
        if is_prime[i] then
            local j = i * i
            while j <= n do
                is_prime[j] = false
                j = j + i
            end
        end
        i = i + 1
    end
    local count = 0
    for k = 2, n do
        if is_prime[k] then count = count + 1 end
    end
    if count ~= 15 then return 0 end
    return 1
end

function test_table_fib()
    local n = 20
    local fib = {}
    fib[1] = 0
    fib[2] = 1
    for i = 3, n do
        fib[i] = fib[i - 1] + fib[i - 2]
    end
    if fib[10] ~= 34 then return 0 end
    if fib[20] ~= 4181 then return 0 end
    return 1
end

function test_table_concat()
    local t = {"hello", " ", "world"}
    local result = ""
    for i = 1, #t do
        result = result .. t[i]
    end
    if result ~= "hello world" then return 0 end
    return 1
end

function test_table_nested()
    local t = {
        a = {1, 2, 3},
        b = {4, 5, 6},
        c = {7, 8, 9}
    }
    local sum = 0
    for _, v in pairs(t.a) do sum = sum + v end
    for _, v in pairs(t.b) do sum = sum + v end
    for _, v in pairs(t.c) do sum = sum + v end
    if sum ~= 45 then return 0 end
    return 1
end
