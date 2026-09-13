function test(x)
    local res = 0
    do
        -- 'x' is shadowed by new local inside do block
        local x = 6
        res = x
    end
    return res
end
