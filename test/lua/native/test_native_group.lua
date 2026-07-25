function test_group_create(pid)
    -- 1. 显式申请/定义一个 group 空间 (group_id = pid)
    local gid = new_native_group(pid)

    -- 2. 申请属于这个 group 的全套 obj
    local player = new_native_obj("player", pid, gid)
    player.name = "Hero_" .. pid

    local bag = new_native_obj("bag", pid * 10, gid)
    bag.gold = 500
    player.bag = bag

    local item = new_native_obj("item", pid * 100, gid)
    item.name = "Excalibur"
    bag.weapon = item

    return player.bag.weapon.name
end

function test_group_destroy(pid)
    -- 3. 一口气释放/注销整个 group 空间的内存对象
    return del_native_group(pid)
end
