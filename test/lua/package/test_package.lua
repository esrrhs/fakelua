function test()
    -- Call Player.AddItem directly via runtime package lookup
    local res1 = Player.AddItem(100, 5) -- 100 + 5 + 1 = 106

    -- Call Bag.UseItem which internally calls Player.AddItem via package lookup
    local res2 = Bag.UseItem(200) -- Player.AddItem(200, 10) = 200 + 10 + 1 = 211

    return res1 + res2 -- 106 + 211 = 317
end
