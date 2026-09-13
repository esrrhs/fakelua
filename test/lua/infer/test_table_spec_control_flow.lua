-- 控制流分支类型冲突退化测试
function test_control_flow(cond)
    local t
    if cond > 0 then
        t = { x = 10 }
    else
        t = { y = 20 }
    end
    local val = t.x
    if val == nil then
        return 0
    else
        return val
    end
end
