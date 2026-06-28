-- pairs 遍历（走 hash fallback）
function test_pairs()
    local t = { x = 10, y = 20 }
    local sum = 0
    for k, v in pairs(t) do
        sum = sum + v
    end
    return sum
end
