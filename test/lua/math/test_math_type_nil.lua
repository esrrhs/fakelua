-- Test math.type with non-numeric arguments (returns nil)

function test_math_type_nil()
    -- nil returns nil
    local r = math.type(nil)
    if r ~= nil then return 1 end

    -- boolean returns nil
    local r2 = math.type(true)
    if r2 ~= nil then return 2 end

    -- string returns nil (not numeric)
    local r3 = math.type("hello")
    if r3 ~= nil then return 3 end

    return 5000
end
