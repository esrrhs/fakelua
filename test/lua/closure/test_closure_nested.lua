function outer(a)
    local b = a * 2
    return function(c)
        local d = c * 3
        return function(e)
            return a + b + c + d + e
        end
    end
end

function test()
    local f1 = outer(5)
    local f2 = f1(4)
    return f2(3)
end
