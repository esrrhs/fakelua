-- 纯算术局部变量链：local x = n + 1; local y = x * 2; return y。
-- 整数特化中，快照将 n+1 和 x*2 均标注为 T_INT，
-- 因此 x 和 y 均声明为 int64_t，return 直接返回 int64_t。
-- test(5) == (5+1)*2 = 12, test(3) == (3+1)*2 = 8.
function test(n)
    local x = n + 1
    local y = x * 2
    return y
end
