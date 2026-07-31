function test_utf8_charpattern()
    -- utf8.charpattern should be a string
    local p = utf8.charpattern
    if type(p) ~= "string" then return 0 end

    -- It should be non-empty
    if #p == 0 then return 0 end

    -- Use it with string.find to match one UTF-8 char
    -- "hello" - first match at position 1
    local pos = string.find("hello", p)
    if pos ~= 1 then return 0 end

    -- Use it with string.gmatch to iterate over UTF-8 chars
    local count = 0
    for ch in string.gmatch("abc", p) do
        count = count + 1
    end
    if count ~= 3 then return 0 end

    return 6000
end
