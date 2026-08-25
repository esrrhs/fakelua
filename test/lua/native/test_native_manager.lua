-- Test NativeObjectManager operations: DestroySingle, Create returning existing,
-- new_native_group without specified id (auto), SetId/SetGroupId
-- C++ side registers: setup_destroy_single, get_destroy_single_obj

function test_destroy_single()
    -- Create objects via Lua API
    local gid = new_native_group()
    local obj1 = new_native_obj(gid, "monster", 101)
    local obj2 = new_native_obj(gid, "monster", 102)
    obj1.hp = 50
    obj2.hp = 80

    -- Getting an existing object via get_native_obj
    local obj1_again = get_native_obj("monster", 101)
    if obj1_again == nil then return 1 end
    if obj1_again.hp ~= 50 then return 2 end

    -- Destroy group
    local count = del_native_group(gid)
    if count ~= 2 then return 3 end

    -- After destroy, should be nil
    local gone = get_native_obj("monster", 101)
    if gone ~= nil then return 4 end

    return 5000
end

function test_new_group_auto_id()
    -- new_native_group without id should auto-assign
    local gid = new_native_group()
    if type(gid) ~= "number" then return 1 end
    if gid <= 0 then return 2 end
    local obj = new_native_obj(gid, "item", 201)
    obj.name = "sword"
    if obj.name ~= "sword" then return 3 end
    del_native_group(gid)
    return 5000
end

function test_create_existing()
    -- Creating an object that already exists should return the same object
    local gid = new_native_group()
    local obj1 = new_native_obj(gid, "hero", 301)
    obj1.level = 5
    local obj2 = new_native_obj(gid, "hero", 301)  -- same type+id -> same object
    if obj2.level ~= 5 then return 1 end
    del_native_group(gid)
    return 5000
end

function test_get_nil_args()
    -- get_native_obj with nil args should return nil
    local r = get_native_obj(nil, 999)
    if r ~= nil then return 1 end
    local r2 = get_native_obj("x", nil)
    if r2 ~= nil then return 2 end
    return 5000
end

function test_access_after_destroy()
    local gid = new_native_group()
    local obj = new_native_obj(gid, "ghost", 901)
    obj.hp = 42
    del_native_group(gid)
    if obj.hp ~= nil then return 1 end
    obj.hp = 1
    if obj.hp ~= nil then return 2 end
    return 5000
end
