local _G = { my = {} }
function _G.my:test(a)
    return a + 1
end

function test(a)
    return a:test(a)
end