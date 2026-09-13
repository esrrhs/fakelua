local function my_compare(x)
    local s = "abc"
    if x < #s then
        return 1
    end
    return 0
end

function test_unsupported_unop(x)
    return my_compare(x)
end
