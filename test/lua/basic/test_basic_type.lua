function test_basic_type()
    if type(nil) ~= "nil" then return 1 end
    if type(true) ~= "boolean" then return 2 end
    if type(false) ~= "boolean" then return 3 end
    if type(42) ~= "number" then return 4 end
    if type(3.14) ~= "number" then return 5 end
    if type("hello") ~= "string" then return 6 end
    if type({}) ~= "table" then return 7 end
    if type(function() end) ~= "function" then return 8 end
    return 5000
end
