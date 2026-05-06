-- n and (n+1): Lua では整数は常に真値なので結果は常に (n+1)。
-- n+1 という加算が数学パラメータ検出のトリガーとなる。
-- 整数特化：CompileNumericExp(n and (n+1)) は n を評価してダミーに格納し、n+1 を返す。
-- test(5) == 6 (5 and 6 = 6)
-- test(0) == 1 (0 は Lua で真値！ 0 and 1 = 1)
-- test(3.5) == 4.5
function test(n)
    return n and (n + 1)
end
