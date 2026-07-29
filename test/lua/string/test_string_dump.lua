function test_string_dump()
    local code = "function add() return 123 + 456 end\nreturn add()"
    local fn = load(code)
    if fn == nil then return 1 end

    local dumped = string.dump(fn)
    if string.sub(dumped, 1, 4) ~= "\027Lua" then return 2 end

    local fn2 = load(dumped)
    if fn2 == nil then return 3 end

    return 700
end
