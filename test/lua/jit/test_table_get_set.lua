function test(a, b)
    local t = { tt = { ttt = a } }
    t.tt.ttt = t.tt.ttt + b
    return t.tt.ttt
end
