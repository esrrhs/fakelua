-- 差分 fuzz 抓到的输入形态：同一块里连着两条 return。
function test_return_twice()
    return 0
    return 0
end
