-- pairs 遍历（走 hash，初始化时双写保证 hash 有数据）
function test_pairs()
    local t = { x = 10, y = 20 }
    t.x = 100
    local sum = 0
    for k, v in pairs(t) do
        sum = sum + v
    end
    return sum
end
