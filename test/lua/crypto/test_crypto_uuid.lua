package "CryptoTest"

-- RFC 4122 v4: 8-4-4-4-12 hex, version nibble 4, variant 8/9/a/b
function test_uuid()
    local a = crypto.uuid()
    local b = crypto.uuid()
    if type(a) ~= "string" or #a ~= 36 then
        print("uuid length:", type(a), tostring(a))
        return 0
    end
    if a == b then
        print("uuid not unique:", a)
        return 0
    end
    -- Lua 模式没有 {n} 量词，按 RFC 段长重复字符类；段间连字符是元字符 '-'
    -- （惰性量词），字面连字符必须写 %-。
    local hex8 = "%x%x%x%x%x%x%x%x"
    local hex4 = "%x%x%x%x"
    local hex3 = "%x%x%x"
    local hex12 = "%x%x%x%x%x%x%x%x%x%x%x%x"
    local uuid_pat = "^" .. hex8 .. "%-" .. hex4 .. "%-4" .. hex3 .. "%-[89ab]" .. hex3 .. "%-" .. hex12 .. "$"
    if not string.find(a, uuid_pat) then
        print("uuid format:", a)
        return 0
    end
    if not string.find(b, uuid_pat) then
        print("uuid format:", b)
        return 0
    end
    return 1
end
