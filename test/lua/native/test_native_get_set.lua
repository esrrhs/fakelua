function test_get_set()
    local p = get_player()
    p.hp = p.hp + 50
    return p.hp
end

-- 版本号测试：只读路径（字段读取 + pairs 快照）不应改变对象版本号
function test_version_read_only()
    local p = get_player()
    local total = p.hp
    for k, v in pairs(p) do
        total = total + 0
    end
    return total
end

-- 版本号测试：快照已建立后的增量写入（NativeSpecSet 的 snapshot_ready 分支）
function test_version_write_after_pairs()
    local p = get_player()
    for _ in pairs(p) do end
    p.hp = p.hp + 1
    return p.hp
end

-- 版本号测试：容器内部写入（MarkModified）需要能传播到父对象
function test_version_container_write()
    local root = get_player()
    local c = container.deque()
    root.bag_container = c
    c:push_back(1)
    return 1
end

-- 版本号测试：容器只读方法不应改变版本号
function test_version_container_read()
    local root = get_player()
    local c = root.bag_container
    if c == nil then return 0 end
    if c:size() ~= 1 then return -1 end
    if c:empty() then return -2 end
    local t = c:to_table()
    return t[1]
end
