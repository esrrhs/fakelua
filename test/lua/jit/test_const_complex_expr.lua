local function double_val(x)
    return x * 2
end

local a = double_val(15)
local b = -a
local c = b + 10
local d = { val = c }

function test()
    return a, b, c, d
end
