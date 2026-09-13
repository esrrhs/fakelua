package "Bag"

function UseItem(id) -- Exported to Bag.UseItem
    -- Zero-require call to Player package via runtime lookup!
    return Player.AddItem(id, 10)
end
