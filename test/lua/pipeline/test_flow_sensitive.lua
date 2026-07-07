-- 流敏感类型: if-else 分支赋不同类型，合并后退化为 T_FLOAT
function test()
    local a
    if 1 then
        a = 1      -- T_INT
    else
        a = 2.5    -- T_FLOAT
    end
    return a       -- 应为 T_FLOAT (meet of int/float)
end
