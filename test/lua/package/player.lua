package "Player"

local BASE_BONUS = 1 -- Constant: compile-time bonus value

function AddItem(id, num) -- Exported to Player.AddItem
    return id + num + BASE_BONUS
end

function RemoveItem(id, num) -- Exported to Player.RemoveItem
    return id - num
end
