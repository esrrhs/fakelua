function test_string_dump()
    local code = "return 123"
    local fn = load(code)
    if fn == nil then return 0 end

    local dumped = string.dump(fn)
    if string.sub(dumped, 1, 4) ~= "\027Lua" then return 0 end

    local fn2 = load(dumped)
    if fn2 == nil then return 0 end

    return 700
end
