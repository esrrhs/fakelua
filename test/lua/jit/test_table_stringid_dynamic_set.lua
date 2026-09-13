function test(k, v)
    local t = { abc = 11 }
    t[k] = v
    return t.abc
end
