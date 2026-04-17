-- Type inference degradation: function call result is T_DYNAMIC,
-- so x = helper() + 2 must degrade to T_DYNAMIC and use CVar arithmetic.
-- helper() returns 1, so test() must return 3.
function helper()
    return 1
end

function test()
    local x = helper() + 2
    return x
end
