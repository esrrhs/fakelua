package "BasicTypeCases"

-- 测试 type 各种类型
function test_type_nil()
    if type(nil) ~= "nil" then return 0 end
    if type(true) ~= "boolean" then return 0 end
    if type(42) ~= "number" then return 0 end
    if type("s") ~= "string" then return 0 end
    if type({}) ~= "table" then return 0 end
    if type(function() end) ~= "function" then return 0 end
    return 1
end
