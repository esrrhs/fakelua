-- C 关键字作数学参数函数名：特化符号必须是 sanitization 后的 C 名（flua_id_int_0），
-- 不能按 AST 名发出 int_0，否则 GCC/TCC 编不过或绑到错误符号。
function int(x)
    return x + 1
end

function test_spec_c_keyword()
    return int(2) + int(3)
end
