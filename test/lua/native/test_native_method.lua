function test_methods(gid)
    local player = new_native_obj(gid, "player", 1)
    player.hp = 100

    -- 冒号语法调用原生成员回调方法
    player:take_damage(30)
    local hp1 = player.hp

    -- 带返回值原生成员方法
    local alive1 = player:is_alive()

    -- 再次扣血，测试逻辑
    player:take_damage(80)
    local hp2 = player.hp
    local alive2 = player:is_alive()

    -- 点号显式传 self
    player.take_damage(player, -50)
    local hp3 = player.hp

    return hp1 + (alive1 and 1000 or 0) + (alive2 and 2000 or 0) + hp3
end
