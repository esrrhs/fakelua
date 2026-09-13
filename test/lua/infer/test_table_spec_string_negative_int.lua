-- 字符串key + 负整数key混合特化
function test_string_negative_int()
    local t = { x = 10, [-1] = 20, [-2] = 30 }
    local a = t.x
    local b = t[-1]
    local c = t[-2]
    if not (a == 10) then return 0 end
    if not (b == 20) then return 0 end
    if not (c == 30) then return 0 end
    t.x = 50
    t[-1] = 60
    if not (t.x == 50) then return 0 end
    if not (t[-1] == 60) then return 0 end
    return 1
end
