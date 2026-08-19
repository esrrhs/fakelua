package "SerializeTest"

-- 布尔 / nil 往返
function test_bool_nil()
    local cases = {true, false, nil}
    for _, v in ipairs(cases) do
        local bin = serialize.encode(v)
        if not bin then return 0 end
        local d = serialize.decode(bin)
        if d ~= v then return 0 end
    end
    return 1
end
