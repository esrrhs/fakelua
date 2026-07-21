function test()
    -- Call Player.AddItem directly via runtime package table
    local res1 = Player.AddItem(100, 5) -- 100 + 5 + 1 = 106

    -- Call Bag.UseItem which internally calls Player.AddItem
    local res2 = Bag.UseItem(200) -- Player.AddItem(200, 10) = 200 + 10 + 1 = 211

    -- Test dynamic hot reloading: replace Player.AddItem at runtime
    Player.AddItem = function(id, num)
        return id * num
    end
    local res3 = Player.AddItem(10, 5) -- 10 * 5 = 50

    return res1 + res2 + res3 -- 106 + 211 + 50 = 367
end
