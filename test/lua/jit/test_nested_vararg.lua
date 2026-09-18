function test_nested_vararg()
    local function outer(...)
        local function inner(...)
            return select('#', ...)
        end
        return select('#', ...), inner(1, 2, 3)
    end
    local a, b = outer(10, 20)
    if a ~= 2 then return 0 end
    if b ~= 3 then return 0 end
    return 1
end
