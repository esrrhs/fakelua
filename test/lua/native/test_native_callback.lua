function run_test()
    local user = { name = "Bob", score = 50 }
    return cpp_process_user(user)
end

function calc()
    return cpp_add_hp(120, 30)
end

-- C++ 侧返回的表必须能在 Lua 侧正常按键索引：这条路径（ViToVar）此前把键值对
-- 直接写在 nodes_[i] 上而不按哈希落槽，导致返回的表在 Lua 里查不到任何键。
function consume_cpp_table()
    local t = cpp_make_table()
    if t.name ~= "Alice" then return 1 end
    if t.level ~= 7 then return 2 end
    if t[1] ~= 10 or t[2] ~= 20 or t[3] ~= 30 then return 3 end
    if #t ~= 3 then return 4 end
    return 9000
end
