-- Test specialization of tables with non-string keys (int, float, bool)
function test_non_string()
    local t = {
        [1] = 10,
        x = 20,
        [true] = 30,
        [2.5] = 40,
        50 -- implicit index 2
    }
    
    -- Read from specialized fields via static keys
    local a = t[1]
    local b = t.x
    local c = t[true]
    local d = t[2.5]
    local e = t[2]
    
    -- Write to specialized fields via static keys
    t[1] = 100
    t.x = 200
    t[true] = 300
    t[2.5] = 400
    t[2] = 500
    
    return a + b + c + d + e + t[1] + t.x + t[true] + t[2.5] + t[2]
end

function test_non_string_dynamic(k1, k2, k3)
    local t = {
        [1] = 10,
        x = 20,
        [true] = 30
    }
    t[k1] = 100
    local a = t[k1]
    local b = t[k2]
    local c = t[k3]
    return a + b + c
end
