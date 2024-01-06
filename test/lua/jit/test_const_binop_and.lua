local a = 21
local b = 3
local c = a and b
local d = false
local e = 2.2
local f = d and e
function test()
    return c, f
end
