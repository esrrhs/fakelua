package "FileLevelTable"

local t = { 10, 20, 30 }
local s = "hello"

function test_file_level_table()
    if t[1] ~= 10 then return 0 end
    if t[2] ~= 20 then return 0 end
    if t[3] ~= 30 then return 0 end
    if s ~= "hello" then return 0 end
    local n = 0
    local sum = 0
    for i, v in ipairs(t) do
        n = n + 1
        sum = sum + v
    end
    if n ~= 3 then return 0 end
    if sum ~= 60 then return 0 end
    return 1
end
