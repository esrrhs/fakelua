local function set_entry(t, k, v)
    FAKELUA_SET_TABLE(t, k, v)
end

function test(key, value)
    local tbl = {}
    set_entry(tbl, key, value)
    return tbl[key]
end
