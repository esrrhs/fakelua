local a = 2
local b = 3
local c = a | b
local d = "-123"
local e = -124
local f = d | e
function test()
    return c, f
end
