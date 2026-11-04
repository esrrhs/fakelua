-- Local multi-assign with multi-return RHS.
function returns_two()
    return 10, 20
end

function test_local_multi_assign()
    local a, b, c = 1, returns_two()
    if a ~= 1 or b ~= 10 or c ~= 20 then return 1 end
    local x, y = returns_two()
    if x ~= 10 or y ~= 20 then return 2 end
    return 0
end
