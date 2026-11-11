-- 足够深的尾递归：若仍按 C 栈一层一层 InterpreterExecute，会在默认栈上炸掉。
function rec(i, acc)
    if i <= 0 then
        return acc
    end
    return rec(i - 1, acc + 1)
end

function test(n)
    return rec(n, 0)
end
