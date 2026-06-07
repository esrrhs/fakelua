function test_shadow_coverage(x)
    local res = 0
    do
        local x = 6
        res = x
    end
    return res
end
