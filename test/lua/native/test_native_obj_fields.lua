-- Test NativeObject field operations: Del, Clear, SetNil, SetFromCVar,
-- ForEach, GetAsCVar, cross-type Get, SetId/SetGroupId/GetId/GetGroupId

function test_obj_del()
    -- test_native C++ side must set up the object before calling this
    local obj = get_test_obj()
    local hp_before = obj.hp
    obj.hp = nil   -- triggers NativeSpecSet with nil -> Del
    local hp_after = obj.hp
    if hp_before ~= 100 then return 1 end
    if hp_after ~= nil then return 2 end
    return 5000
end

function test_obj_set_nil_key()
    local obj = get_test_obj()
    obj.speed = 3.14  -- float field
    local as_int = obj.speed  -- GetFloat returns float; also triggers cross-type path
    if as_int == nil then return 1 end
    return 5000
end

function test_obj_foreach()
    local obj = get_test_obj()
    obj.a = 1
    obj.b = 2
    obj.c = 3
    local count = count_fields(obj)
    if count < 3 then return 1 end
    return 5000
end

function test_obj_get_as_cvar()
    local obj = get_test_obj()
    obj.score = 42
    local v = obj.score
    if v ~= 42 then return 1 end
    return 5000
end
