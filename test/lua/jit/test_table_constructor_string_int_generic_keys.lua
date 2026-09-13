function test(a_key, int_key, bool_key)
    local t = { ["a"] = 1, [2] = 2, [true] = 3 }
    return t[a_key] + t[int_key] + t[bool_key]
end
