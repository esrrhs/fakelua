package "Player"

local log_count = 0 -- Private to player.lua

function AddItem(id, num) -- Exported to Player.AddItem
    log_count = log_count + 1
    return id + num + log_count
end

function GetLogCount() -- Exported to Player.GetLogCount
    return log_count
end
