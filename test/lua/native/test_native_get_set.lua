function test_get_set()
    local p = get_player()
    p.hp = p.hp + 50
    return p.hp
end
