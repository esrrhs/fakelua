-- Shadowing a global const inside a function scope should be rejected
local MY_CONST = 10
function test()
    local MY_CONST = 20
end
