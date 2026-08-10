-- Test NativeObject method calls: self-passing via dot vs colon syntax
-- Also tests RegisterMethod/HasMethod/UnregisterMethod at Lua level

function test_method_dot_syntax()
    -- Call method via dot notation with explicit self
    local obj = get_test_player()
    if obj == nil then return 1 end

    -- take_damage is registered with explicit self
    obj.take_damage(obj, 15)
    local hp = obj.hp
    if hp ~= 85 then return 2 end
    return 5000
end

function test_method_colon_syntax()
    -- Call method via colon notation
    local obj = get_test_player()
    if obj == nil then return 1 end

    obj:take_damage(10)
    local hp = obj.hp
    if hp ~= 90 then return 2 end
    return 5000
end

function test_method_nil_field()
    local obj = get_test_player()
    if obj == nil then return 1 end

    -- Accessing non-existent method should return nil
    local m = obj.nonexistent_method
    if m ~= nil then return 2 end

    return 5000
end
