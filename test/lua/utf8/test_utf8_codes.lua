function test_utf8_codes()
    local acc = ""
    local n = 0
    for p, c in utf8.codes("AB") do
        n = n + 1
        acc = acc .. string.char(c)
        if n == 1 and p ~= 1 then return 0 end
        if n == 2 and p ~= 2 then return 0 end
    end
    if n ~= 2 then return 0 end
    if acc ~= "AB" then return 0 end

    -- 超过 SSO 的字符串走 heap 缓冲；arena 必须登记析构，否则 Reset 后 UAF。
    local long = string.rep("B", 64)
    local ln = 0
    for p, c in utf8.codes(long) do
        ln = ln + 1
        if c ~= 66 then return 0 end
        if p ~= ln then return 0 end
    end
    if ln ~= 64 then return 0 end

    local text = "AB"
    local cp_a = utf8.codepoint(text, 1, 1)
    local cp_b = utf8.codepoint(text, 2, 2)
    if cp_a ~= 65 then return 0 end
    if cp_b ~= 66 then return 0 end

    return 6000
end
