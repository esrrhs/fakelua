local function my_compare(x, y, z)
    local dummy1 = y + 1.0
    local dummy2 = z + 1.0
    if x < (y or z) then
        return 1
    end
    return 0
end

function test_unsupported(x, y, z)
    return my_compare(x, y, z)
end
