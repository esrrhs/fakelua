-- n or 2 は整数特化では常に n（n は Lua で常に真値：0 を含む）を返す。
-- n * 3 という乗算が数学パラメータ検出のトリガーとなる。
-- 整数特化：local y = n or 2 は int64_t y = n に直接コンパイルされる（CVar 装箱なし）。
-- test(5) == 15 (5 or 2 = 5, 5*3 = 15)
-- test(0) == 0  (0 は Lua で真値！ 0 or 2 = 0, 0*3 = 0)
-- test(3.5) == 10.5
function test(n)
    local y = n or 2
    return y * 3
end
