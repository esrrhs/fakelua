function test(...)
    local function helper(x)
        return x
    end
    return helper(...)
end
