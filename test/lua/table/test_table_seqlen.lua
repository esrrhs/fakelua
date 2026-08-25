-- 覆盖 # 运算符（连续整数键前缀长度）在各种写入模式下的取值。
-- VarTable 会缓存该长度以避免每次 # 都做 O(n) 扫描，本用例针对缓存的更新与失效路径：
-- 跨越 quick_data_(8 槽) 到哈希模式的迁移、乱序写入后向上吸收、nil 删除截断、
-- 浮点键归一化、以及与字符串键互不干扰。
-- 失败时返回出错的步骤号，全部通过返回 8100。

function test_table_seqlen()
    -- 1. 纯追加，跨越 8 槽边界
    local t1 = {}
    for i = 1, 20 do
        t1[i] = i
        if #t1 ~= i then return 1 end
    end

    -- 2. 乱序写入：补齐最后一个空洞时应一次性吸收后续已存在的键
    local t2 = {}
    t2[3] = "c"
    if #t2 ~= 0 then return 2 end
    t2[2] = "b"
    if #t2 ~= 0 then return 3 end
    t2[1] = "a"
    if #t2 ~= 3 then return 4 end

    -- 3. 倒序写入 2..12 后补上 1，需跨越 8 槽边界一次性吸收
    local t3 = {}
    for i = 12, 2, -1 do t3[i] = i end
    if #t3 ~= 0 then return 5 end
    t3[1] = 1
    if #t3 ~= 12 then return 6 end

    -- 4. 中间挖洞后再补回
    local t4 = {}
    for i = 1, 12 do t4[i] = i end
    if #t4 ~= 12 then return 7 end
    t4[5] = nil
    if #t4 ~= 4 then return 8 end
    t4[5] = 5
    if #t4 ~= 12 then return 9 end

    -- 5. 从尾部逐个删除
    local t5 = {}
    for i = 1, 12 do t5[i] = i end
    for i = 12, 1, -1 do
        t5[i] = nil
        if #t5 ~= i - 1 then return 10 end
    end

    -- 6. 浮点键归一化：t[2.0] 等价于 t[2]
    local t6 = {}
    t6[1] = "a"
    t6[2.0] = "b"
    t6[3.0] = "c"
    if #t6 ~= 3 then return 11 end
    if t6[2] ~= "b" or t6[3] ~= "c" then return 12 end

    -- 7. 字符串键不影响整数键前缀长度
    local t7 = {}
    for i = 1, 10 do t7[i] = i end
    t7["name"] = "x"
    if #t7 ~= 10 then return 13 end
    t7["name"] = nil
    if #t7 ~= 10 then return 14 end

    -- 8. 表构造器字面量跨 8 槽
    local t8 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12}
    if #t8 ~= 12 then return 15 end
    t8[13] = 13
    if #t8 ~= 13 then return 16 end

    -- 9. 覆盖前缀内已有元素不改变长度
    local t9 = {}
    for i = 1, 12 do t9[i] = i end
    t9[6] = 600
    if #t9 ~= 12 then return 17 end
    if t9[6] ~= 600 then return 18 end

    -- 10. 在前缀之外写入不延长长度（留有空洞）
    local t10 = {}
    for i = 1, 10 do t10[i] = i end
    t10[20] = 20
    if #t10 ~= 10 then return 19 end
    if t10[20] ~= 20 then return 20 end

    -- 11. 特化哈希表：spec_count 是字段个数，不是数组长度
    local th = {a = 1, b = 2}
    if #th ~= 0 then return 21 end
    table.insert(th, "x")
    if th[1] ~= "x" or #th ~= 1 then return 22 end

    -- 12. 特化混合表：# 应停在连续整数前缀，不能把 name 算进去
    local tm = {1, 2, name = "x"}
    if #tm ~= 2 then return 23 end
    if tm.name ~= "x" then return 24 end
    table.insert(tm, 3)
    if #tm ~= 3 or tm[3] ~= 3 then return 25 end

    return 8100
end
