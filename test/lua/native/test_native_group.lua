function test_group_create(pid)
    -- 1. 申请分配一个唯一 group 空间
    local gid = new_native_group()

    -- 2. 申请属于这个 group 的全套 obj
    local player = new_native_obj(gid, "player", pid)
    player.name = "Hero_" .. pid

    local bag = new_native_obj(gid, "bag", pid * 10)
    bag.gold = 500
    player.bag = bag

    local item = new_native_obj(gid, "item", pid * 100)
    item.name = "Excalibur"
    bag.weapon = item

    return gid
end

function test_group_destroy(gid)
    -- 3. 一口气释放/注销整个 group 空间的内存对象
    return del_native_group(gid)
end
