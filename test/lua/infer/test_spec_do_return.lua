-- return 语句位于 do...end 块内部的函数。
-- AllPathsReturn 和 CollectReturnExps 现在能正确处理 Block（do...end）节点，
-- 使特化函数的返回类型被推断为 int64_t 而非 CVar。
-- test(10) == 11。
function test(n)
    do
        return n + 1
    end
end
