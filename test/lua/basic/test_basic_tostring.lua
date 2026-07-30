function test_basic_tostring()
    if tostring(nil) ~= "nil" then return 1 end
    if tostring(true) ~= "true" then return 2 end
    if tostring(false) ~= "false" then return 3 end
    if tostring(42) ~= "42" then return 4 end
    if tostring("hello") ~= "hello" then return 5 end
    if tostring(-100) ~= "-100" then return 6 end
    return 5000
end
