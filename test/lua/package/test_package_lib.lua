package "Pkg"

local SECRET = 7 -- Chunk-local constant: verifies constant folding inside a package function

function add(a, b) -- Exported to Pkg.add
    return a + b
end

function triple() -- Exported to Pkg.triple (multiple return values)
    return 1, 2, 3
end

function sum(...) -- Exported to Pkg.sum (varargs)
    local t = {...}
    local total = 0
    for i = 1, #t do
        total = total + t[i]
    end
    return total
end

function with_const(x) -- Exported to Pkg.with_const (uses chunk-local constant)
    return x * SECRET
end

function call_global(n) -- Exported to Pkg.call_global (package -> global direction)
    return helper(n)
end

local function hidden(x) -- NOT exported (local function)
    return x + 100
end

function use_hidden(x) -- Exported: internally calls the non-exported hidden()
    return hidden(x)
end
