function test_math_spec_mixed(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)
    local sum = p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8
    if p9 == p10 then
        return sum
    else
        return sum + 1
    end
end
