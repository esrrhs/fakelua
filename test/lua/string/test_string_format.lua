function test_string_format()
    local s1 = string.format("hello %s %d", "world", 100)
    if s1 ~= "hello world 100" then return 0 end

    local s2 = string.format("%.2f", 3.14159)
    if s2 ~= "3.14" then return 0 end

    local s3 = string.format("%q", "foo\"bar")
    if s3 ~= "\"foo\\\"bar\"" then return 0 end

    return 600
end
