function test_nested()
    -- 创建 player 对象和 bag 对象（均关联归属于 2001 玩家组）
    local player = new_native_obj(2001, "player", 2001)
    local bag = new_native_obj(2001, "bag", 3001)

    -- 在 Lua 侧进行对象嵌套赋值：player.bag = bag
    player.bag = bag

    -- 在 Lua 侧链式读写嵌套对象的字段：player.bag.gold = 999
    player.bag.gold = 999
    player.bag.capacity = 50

    -- 返回读取的值
    return player.bag.gold + player.bag.capacity
end

function test_nested_fetch()
    -- 获取已有 player 对象，继续测试链式读取 player.bag.gold
    local player = get_native_obj("player", 2001)
    if player == nil or player.bag == nil then
        return 0
    end

    -- 消耗 100 gold
    player.bag.gold = player.bag.gold - 100
    return player.bag.gold
end
