function test_basic_next()
    -- 非空表返回至少一个键
    local t2 = {a = 1, b = 2, c = 3}
    local k, v = next(t2)
    if k == nil then return 1 end

    -- 遍历所有键值对
    local count_pairs = 0
    local k1 = nil
    while true do
        local key, val = next(t2, k1)
        if key == nil then break end
        count_pairs = count_pairs + 1
        k1 = key
    end
    if count_pairs ~= 3 then return 2 end

    -- 空表返回 nil
    local ek, ev = next({})
    if ek ~= nil then return 3 end

    -- 整数 key 的表
    local t3 = {10, 20, 30}
    local k2, v2 = next(t3)
    if k2 ~= 1 or v2 ~= 10 then return 4 end

    local k3, v3 = next(t3, 2)
    if k3 ~= 3 or v3 ~= 30 then return 5 end

    -- 超出范围返回 nil
    local k4, v4 = next(t3, 3)
    if k4 ~= nil then return 6 end

    -- 动态拼接 key (VAR_STRING 与 VAR_STRINGID 跨类型比对)
    local dyn_key = "a" .. ""
    local nk, nv = next(t2, dyn_key)
    if nk == nil then return 7 end

    return 5000
end
