local function my_add(x)
    if x + 1.0 then
        return 2.0
    end
    return 0.0
end

function test_cond(x)
    return my_add(x)
end
