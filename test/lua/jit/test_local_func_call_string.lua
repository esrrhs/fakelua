local function inner(t)
    return t.."_"..t
end

function test(a, b)
    return inner "test"
end
