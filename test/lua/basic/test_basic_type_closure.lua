-- Test basic type() function with closure type
-- Covers the VarType::Closure branch in type() which returns "function"

function some_global_function()
    return 42
end

function test_type_closure()
    local f = function() return 1 end
    if type(f) ~= "function" then return 1 end

    -- Named function
    local function g() return 2 end
    if type(g) ~= "function" then return 2 end

    -- Reference to a top-level Lua function (compiled to a real C function
    -- pointer, so it can be used as a first-class value like other closures)
    local h = some_global_function
    if type(h) ~= "function" then return 3 end
    if h() ~= 42 then return 4 end

    return 5000
end

function test_tonumber_with_base()
    -- tonumber with base 16
    local r = tonumber("ff", 16)
    if r ~= 255 then return 1 end

    -- tonumber with base 2
    local r2 = tonumber("1010", 2)
    if r2 ~= 10 then return 2 end

    -- tonumber with base 8
    local r3 = tonumber("17", 8)
    if r3 ~= 15 then return 3 end

    -- invalid character for base
    local r4 = tonumber("g", 16)
    if r4 ~= nil then return 4 end

    -- tonumber("0xff") hex auto-detect
    local r5 = tonumber("0xff")
    if r5 ~= 255 then return 5 end

    return 5000
end

function test_tonumber_with_base_negative()
    -- tonumber("-ff", 16)
    local r = tonumber("-ff", 16)
    if r ~= -255 then return 1 end

    -- tonumber("+7f", 16)
    local r2 = tonumber("+7f", 16)
    if r2 ~= 127 then return 2 end

    return 5000
end

function test_tonumber_with_base_invalid()
    -- base out of range
    local r = tonumber("10", 1)
    if r ~= nil then return 1 end

    -- base 37 (too large)
    local r2 = tonumber("10", 37)
    if r2 ~= nil then return 2 end

    return 5000
end

function test_type_nil_multi_table()
    -- type nil
    if type(nil) ~= "nil" then return 1 end
    -- type table
    if type({}) ~= "table" then return 2 end
    -- type bool
    if type(true) ~= "boolean" then return 3 end
    if type(false) ~= "boolean" then return 4 end
    return 5000
end
