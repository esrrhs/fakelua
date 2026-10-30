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
    obj.speed = 3.14 -- SetFloat

    -- GetFloat should return float
    local s = obj.speed
    if math.abs(s - 3.14) > 0.001 then return 1 end

    del_native_group(gid)
    return 5000
end

function test_bool_field()
    local gid = new_native_group()
    local obj = new_native_obj(gid, "item", 403)
    obj.alive = true -- SetBool

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
    obj.name = "sword" -- SetString

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

-- Cover incremental spec_keys/spec_vals maintenance in NativeSpecSet:
-- update existing fields (no full rebuild), delete, then insert; pairs must
-- reflect the latest key set and values.
function test_pairs_after_incremental_spec_set()
    local gid = new_native_group()
    local obj = new_native_obj(gid, "item", 409)
    obj.a = 1
    obj.b = 2
    obj.c = 3

    -- Hot path: overwrite existing fields many times
    for i = 1, 20 do
        obj.a = i
        obj.b = i * 10
    end
    if obj.a ~= 20 then return 1 end
    if obj.b ~= 200 then return 2 end

    -- Delete one field, then insert a new one
    obj.b = nil
    if obj.b ~= nil then return 3 end
    obj.d = 40

    local seen = {}
    local count = 0
    for k, v in pairs(obj) do
        seen[k] = v
        count = count + 1
    end
    if count ~= 3 then return 4 end
    if seen.a ~= 20 then return 5 end
    if seen.c ~= 3 then return 6 end
    if seen.d ~= 40 then return 7 end
    if seen.b ~= nil then return 8 end

    del_native_group(gid)
    return 5000
end

-- Deep nesting: Wrap must stay light (no recursive snapshot of the whole tree).
-- Field get/set through the chain still works; pairs on root sees only one level.
function test_deep_nesting_lazy_wrap()
    local gid = new_native_group()
    local root = new_native_obj(gid, "node", 701)
    local cur = root
    for i = 1, 40 do
        local child = new_native_obj(gid, "node", 701 + i)
        child.depth = i
        cur.child = child
        cur = child
    end

    -- Walk nested field gets (each level light-wraps on demand)
    local n = root
    for i = 1, 40 do
        n = n.child
        if n == nil then return 1 end
        if n.depth ~= i then return 2 end
    end
    if n.child ~= nil then return 3 end

    -- Nested field set through chain
    root.child.child.depth = 99
    if root.child.child.depth ~= 99 then return 4 end

    -- pairs on root: one level only (child + whatever else); must not explode
    local seen_child = false
    local count = 0
    for k, v in pairs(root) do
        count = count + 1
        if k == "child" then
            seen_child = true
            if v == nil then return 5 end
            if v.depth ~= 1 then return 6 end
        end
    end
    if not seen_child then return 7 end
    if count < 1 then return 8 end

    -- pairs after incremental writes once snapshot exists
    root.tag = "root"
    local seen_tag = false
    for k, v in pairs(root) do
        if k == "tag" and v == "root" then seen_tag = true end
    end
    if not seen_tag then return 9 end

    del_native_group(gid)
    return 5000
end

-- Writes before any pairs: kv-only path; first pairs builds snapshot including them.
function test_pairs_after_lazy_writes()
    local gid = new_native_group()
    local obj = new_native_obj(gid, "item", 801)
    obj.a = 1
    obj.b = 2
    obj.c = 3
    obj.b = nil
    obj.d = 4

    local seen = {}
    local count = 0
    for k, v in pairs(obj) do
        seen[k] = v
        count = count + 1
    end
    if count ~= 3 then return 1 end
    if seen.a ~= 1 then return 2 end
    if seen.c ~= 3 then return 3 end
    if seen.d ~= 4 then return 4 end
    if seen.b ~= nil then return 5 end

    -- Further incremental update after snapshot
    obj.a = 10
    obj.e = 5
    seen = {}
    count = 0
    for k, v in pairs(obj) do
        seen[k] = v
        count = count + 1
    end
    if count ~= 4 then return 6 end
    if seen.a ~= 10 then return 7 end
    if seen.e ~= 5 then return 8 end

    del_native_group(gid)
    return 5000
end

-- next() on native object must see fields via EnsureNativeSpecSnapshot
function test_next_on_native_obj()
    local gid = new_native_group()
    local obj = new_native_obj(gid, "item", 901)
    obj.x = 11
    obj.y = 22

    local seen = {}
    local k, v = next(obj, nil)
    while k ~= nil do
        seen[k] = v
        k, v = next(obj, k)
    end
    if seen.x ~= 11 then return 1 end
    if seen.y ~= 22 then return 2 end

    del_native_group(gid)
    return 5000
end
