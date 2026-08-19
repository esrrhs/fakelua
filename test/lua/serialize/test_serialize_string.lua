package "SerializeTest"

-- 字符串往返：空串、普通串、含二进制 0 的串
-- 字典去重：同一字符串出现多次应被复用
function test_string()
    local cases = {"", "hello", "a longer string for testing", string.char(0, 1, 2) .. "binary"}
    for _, v in ipairs(cases) do
        local bin = serialize.encode(v)
        if not bin then return 0 end
        local d = serialize.decode(bin)
        if d ~= v then return 0 end
    end

    -- 字典去重验证：包含两个相同字符串的表，第二次应走 STR_REF
    local t = {name = "repeat", label = "repeat", unique = "other"}
    local bin = serialize.encode(t)
    if not bin then return 0 end
    local d = serialize.decode(bin)
    if d.name ~= "repeat" then return 0 end
    if d.label ~= "repeat" then return 0 end
    if d.unique ~= "other" then return 0 end
    return 1
end
