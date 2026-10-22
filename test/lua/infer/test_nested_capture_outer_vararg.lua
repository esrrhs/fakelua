-- Nested non-vararg functions may lexically capture the outer function's ...;
-- a nested vararg function uses its own ... .

function test_capture_outer_vararg()
    local function outer(...)
        local function inner()
            return select('#', ...)
        end
        return inner()
    end
    local n = outer(10, 20, 30)
    if n ~= 3 then return 0 end
    return 1
end

function test_nested_own_vararg()
    local function outer(...)
        local outer_n = select('#', ...)
        local function inner(...)
            return select('#', ...)
        end
        local inner_n = inner(1, 2)
        if outer_n ~= 3 then return 0 end
        if inner_n ~= 2 then return 0 end
        return 1
    end
    return outer(9, 8, 7)
end

function test_funcdef_capture_outer_vararg()
    local function outer(...)
        local a = select(1, ...)
        local inner = function()
            return select('#', ...)
        end
        local n = inner()
        if a ~= 42 then return 0 end
        if n ~= 2 then return 0 end
        return 1
    end
    return outer(42, 43)
end
