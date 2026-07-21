function test()
    -- Call Player.AddItem directly via runtime package table
    local res1 = Player.AddItem(100, 5) -- 100 + 5 + 1 = 106
    -- Call Bag.UseItem which calls Player.AddItem
    local res2 = Bag.UseItem(200) -- 200 + 10 + 2 = 212
    -- Verify private local variable log_count is 2
    local count = Player.GetLogCount() -- 2

    -- Test dynamic hot reloading / closure replacement on package table:
    Player.AddItem = function(id, num)
        return id * num
    end
    local res3 = Bag.UseItem(20) -- Bag.UseItem calls updated Player.AddItem(20, 10) -> 200

    return res1 + res2 + count + res3 -- 106 + 212 + 2 + 200 = 520
end
