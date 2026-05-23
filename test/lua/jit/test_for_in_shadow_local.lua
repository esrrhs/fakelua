-- Shadow the for-in loop variable with a local of the same name inside the body.
-- t = {10, 20, 30}: keys k=1,2,3 in iteration order.
-- inner_k = k*2 = 2,4,6  => sum=12
function test()
    local t = {10, 20, 30}
    local sum = 0
    for k, v in pairs(t) do
        local k = k * 2
        sum = sum + k
    end
    return sum
end
