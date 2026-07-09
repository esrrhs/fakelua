-- Shape 字面量: {b=1, c=2.0}
-- 期望: 封闭 record, 字段 b:T_INT, c:T_FLOAT
function test()
    local a = {b = 1, c = 2.0}
    return a.b + a.c
end
