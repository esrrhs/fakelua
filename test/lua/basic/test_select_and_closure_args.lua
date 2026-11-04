-- select(), many-arg calls, and multi-return flattening.
function many_args(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10)
    return a1+a2+a3+a4+a5+a6+a7+a8+a9+a10
end

function test_select_and_closure_args()
    if select("#", 1, 2, 3) ~= 3 then return 1 end
    if select(2, 10, 20, 30) ~= 20 then return 2 end
    local f = function(...)
        return select("#", ...)
    end
    if f(1, 2, 3, 4) ~= 4 then return 3 end
    if many_args(1,2,3,4,5,6,7,8,9,10) ~= 55 then return 4 end
    -- closure call with multi returns flattened
    local function ret_multi() return 1, 2, 3 end
    local function take3(a, b, c) return a + b + c end
    if take3(ret_multi()) ~= 6 then return 5 end
    return 0
end
