local function make_table(s)
    local t = { [#s] = 42 }
    return t[#s]
end

function test_construct(s)
    return make_table(s)
end
