function test_utf8_len()
    -- ASCII string
    local l1 = utf8.len("hello")
    if l1 ~= 5 then return 1 end

    -- Mixed: "aé€" = 1 + 1 + 1 = 3 UTF-8 chars (but different byte lengths)
    local l2 = utf8.len("aé€")
    if l2 ~= 3 then return 2 end

    -- Empty string
    local l3 = utf8.len("")
    if l3 ~= 0 then return 3 end

    -- With byte range (i=2, j=4 for "hello" -> 'e', 'l', 'l' -> 3 chars)
    local l4 = utf8.len("hello", 2, 4)
    if l4 ~= 3 then return 4 end

    -- 4-byte char
    local l5 = utf8.len("𝄞")
    if l5 ~= 1 then return 5 end

    -- 基于字节索引范围计算字符数：
    -- "aé€" 中 'a'(byte 1), 'é'(bytes 2-3), '€'(bytes 4-6)
    -- 从字节位置 2 到 6 应包含 'é' 和 '€' 两个字符
    local l6 = utf8.len("aé€", 2, 6)
    if l6 ~= 2 then return 6 end

    -- 数字隐式转字符串
    if utf8.len(12345) ~= 5 then return 7 end

    -- 下标 0 不是从末尾倒数（Lua u_posrelat 只把负数当相对下标）。
    -- 经 is_nil 走 CVar，避免特化把 nil 拆成 int 0。
    local function is_nil(v)
        return v == nil
    end
    if not is_nil(utf8.len("abc", 0)) then return 8 end

    return 6000
end
