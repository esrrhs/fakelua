function test(...)
    local f = function(x)
        return x
    end
    return f(...)
end
