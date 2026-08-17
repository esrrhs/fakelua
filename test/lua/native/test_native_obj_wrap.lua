-- Test NativeObject wrapping/unwrapping: NativeFieldToCVar with Object,
-- CVarToNativeField with Table, RefreshSpecKeys, SetFromCVar, GetAsCVar

function test_wrap_object_field()
    local gid = new_native_group()
    local parent = new_native_obj(gid, "player", 501)
    local bag = new_native_obj(gid, "bag", 502)
    bag.gold = 999
    parent.bag = bag  -- triggers SetObject -> NativeField::Kind::Object

    -- Verify bag.gold is accessible through parent
    if parent.bag == nil then return 1 end
    if parent.bag.gold ~= 999 then return 2 end

    -- Modify nested object through wrapper
    parent.bag.gold = 500
    if parent.bag.gold ~= 500 then return 3 end

    del_native_group(gid)
    return 5000
end

function test_wrap_empty_spec_keys()
    local gid = new_native_group()
    local obj = new_native_obj(gid, "empty", 503)  -- no fields set

    -- pairs() on an empty native object should iterate 0 times
    local count = 0
    for k, v in pairs(obj) do
        count = count + 1
    end
    if count ~= 0 then return 1 end

    del_native_group(gid)
    return 5000
end

function test_wrap_set_from_cvar()
    local gid = new_native_group()
    local obj = new_native_obj(gid, "cvartest", 504)

    -- Setting various Lua types that go through CVarToNativeField
    obj.int_val = 42          -- SetInt path
    obj.float_val = 3.14      -- SetFloat path
    obj.bool_val = true       -- SetBool path
    obj.str_val = "hello"     -- SetString path
    obj.nil_val = nil         -- Remove field path

    -- Verify round-trip
    if obj.int_val ~= 42 then return 1 end
    local fv = obj.float_val
    if math.abs(fv - 3.14) > 0.001 then return 2 end
    if obj.bool_val ~= true then return 3 end
    if obj.str_val ~= "hello" then return 4 end
    if obj.nil_val ~= nil then return 5 end

    del_native_group(gid)
    return 5000
end

function test_wrap_get_as_cvar()
    local gid = new_native_group()
    local obj = new_native_obj(gid, "gettest", 505)
    obj.hp = 100
    obj.name = "warrior"
    obj.alive = true

    -- All field types should be accessible
    if obj.hp ~= 100 then return 1 end
    if obj.name ~= "warrior" then return 2 end
    if obj.alive ~= true then return 3 end

    -- Missing field returns nil
    local x = obj.non_existent
    if x ~= nil then return 4 end

    del_native_group(gid)
    return 5000
end
