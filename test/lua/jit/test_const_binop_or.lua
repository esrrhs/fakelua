local a = 21
local b = 3
local c = a or b
local d = false
local e = 2.2
local f = d or e
function test()
    return c, f
end
