-- Tail-call correctness. Depth stays modest so INTERP (no TCO) does not overflow the C stack.

function tail_sum(x, acc)
    if x <= 0 then
        return acc
    end
    return tail_sum(x - 1, acc + x)
end

function test_tail_sum()
    if tail_sum(0, 0) ~= 0 then return 0 end
    if tail_sum(1, 0) ~= 1 then return 0 end
    if tail_sum(10, 0) ~= 55 then return 0 end
    if tail_sum(100, 0) ~= 5050 then return 0 end
    return 1
end

function even(n)
    if n == 0 then return true end
    return odd(n - 1)
end

function odd(n)
    if n == 0 then return false end
    return even(n - 1)
end

function test_tail_even_odd()
    if even(0) ~= true then return 0 end
    if odd(0) ~= false then return 0 end
    if even(10) ~= true then return 0 end
    if odd(11) ~= true then return 0 end
    if even(99) ~= false then return 0 end
    return 1
end

function ident(x)
    return x
end

function tail_through(x)
    return ident(x + 1)
end

function test_tail_other_func()
    if tail_through(41) ~= 42 then return 0 end
    return 1
end

function two_rets(a, b)
    return a, b
end

function tail_multi(x)
    return two_rets(x, x + 1)
end

function test_tail_multi_ret()
    local a, b = tail_multi(7)
    if a ~= 7 then return 0 end
    if b ~= 8 then return 0 end
    return 1
end

function add1(a)
    return a
end

function add3(a, b, c)
    return a + b + c
end

function tail_to_add1(x)
    return add1(x)
end

function tail_to_add3(x)
    return add3(x, 2, 3)
end

function test_tail_arity()
    if tail_to_add1(5) ~= 5 then return 0 end
    if tail_to_add3(5) ~= 10 then return 0 end
    return 1
end

function tail_count(...)
    local n = select('#', ...)
    if n <= 1 then return n end
    return tail_count(select(2, ...))
end

function test_tail_vararg()
    if tail_count() ~= 0 then return 0 end
    if tail_count(1) ~= 1 then return 0 end
    if tail_count(1, 2, 3, 4, 5) ~= 1 then return 0 end
    return 1
end

function test_tail_closure()
    local function walk(n, acc)
        if n <= 0 then return acc end
        return walk(n - 1, acc + n)
    end
    if walk(20, 0) ~= 210 then return 0 end
    return 1
end
