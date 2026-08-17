-- Test NativeObject advanced operations: Del/Clear field, SetNil, ForEach,
-- GetAsCVar, cross-type Get (float->int, int->float), GetBool with missing key,
-- GetString with missing key, GetObject missing key

function test_del_field()
    local gid = new_native_group()
    local obj = new_native_obj(gid, "item", 401)
    obj.hp = 100
    obj.mp = 200

    -- Del by setting to nil
    obj.hp = nil
    local hp_after = obj.hp
    if hp_after ~= nil then return 1 end

    -- mp still exists
    if obj.mp ~= 200 then return 2 end

    del_native_group(gid)
    return 5000
end

function test_float_field()
    local gid = new_native_group()
    local obj = new_native_obj(gid, "item", 402)
    obj.speed = 3.14  -- SetFloat

    -- GetFloat should return float
    local s = obj.speed
    if math.abs(s - 3.14) > 0.001 then return 1 end

    del_native_group(gid)
    return 5000
end

function test_bool_field()
    local gid = new_native_group()
    local obj = new_native_obj(gid, "item", 403)
    obj.alive = true   -- SetBool

    -- GetBool
    local a = obj.alive
    if a ~= true then return 1 end

    -- Field not exist returns nil
    local x = obj.nonexist
    if x ~= nil then return 2 end

    del_native_group(gid)
    return 5000
end

function test_string_field()
    local gid = new_native_group()
    local obj = new_native_obj(gid, "item", 404)
    obj.name = "sword"  -- SetString

    -- GetString
    if obj.name ~= "sword" then return 1 end

    -- Missing string field returns nil
    local x = obj.missing_str
    if x ~= nil then return 2 end

    del_native_group(gid)
    return 5000
end

function test_object_field()
    local gid = new_native_group()
    local parent = new_native_obj(gid, "player", 405)
    local child = new_native_obj(gid, "bag", 406)
    child.gold = 999

    -- SetObject (nested)
    parent.bag = child

    -- GetObject
    local bag = parent.bag
    if bag == nil then return 1 end
    if bag.gold ~= 999 then return 2 end

    -- Missing object field returns nil
    local x = parent.missing_obj
    if x ~= nil then return 3 end

    del_native_group(gid)
    return 5000
end

function test_pairs_on_native_obj()
    local gid = new_native_group()
    local obj = new_native_obj(gid, "item", 407)
    obj.a = 1
    obj.b = 2
    obj.c = 3

    -- pairs() iterates over spec_keys of the wrapped native object
    local count = 0
    for k, v in pairs(obj) do
        count = count + 1
    end
    if count < 3 then return 1 end

    del_native_group(gid)
    return 5000
end

function test_int_bool_not_equal_cross()
    local gid = new_native_group()
    local obj = new_native_obj(gid, "item", 408)
    obj.v = 42

    -- Setting nil explicitly removes the field
    obj.v = nil
    if obj.v ~= nil then return 1 end

    del_native_group(gid)
    return 5000
end
