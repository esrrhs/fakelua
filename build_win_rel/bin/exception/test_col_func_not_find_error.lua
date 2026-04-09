local _G = { my = { test1 = "xxx" } }
function _G.my:test(a)
    return a + 1
end

function test(a)
    return _G.my:test1(a)
end