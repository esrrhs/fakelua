function test_string_case()
    local s = "Hello World"
    if string.lower(s) ~= "hello world" then return 0 end
    if string.upper(s) ~= "HELLO WORLD" then return 0 end
    if string.reverse("abc") ~= "cba" then return 0 end
    return 400
end
