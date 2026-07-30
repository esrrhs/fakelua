function test_string_sub()
    local s = "hello world"
    if string.sub(s, 1, 5) ~= "hello" then return 0 end
    if string.sub(s, 7) ~= "world" then return 0 end
    if string.sub(s, -5) ~= "world" then return 0 end
    return 200
end
