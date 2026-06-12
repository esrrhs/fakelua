-- Lua 5.4 function and recursion tests
-- FakeLua only supports global function definitions (not local function)

function fact(n)
    if n <= 1 then return 1 end
    return n * fact(n - 1)
end

function fib(n)
    if n <= 1 then return n end
    return fib(n - 1) + fib(n - 2)
end

function gcd_func(a, b)
    while b ~= 0 do
        a, b = b, a % b
    end
    return a
end

function sum_range(n, acc)
    if n == 0 then return acc end
    return sum_range(n - 1, acc + n)
end

function power(base, exp)
    if exp == 0 then return 1 end
    if exp % 2 == 0 then
        local half = power(base, exp // 2)
        return half * half
    else
        return base * power(base, exp - 1)
    end
end

function add8(a, b, c, d, e, f, g, h)
    return a + b + c + d + e + f + g + h
end

function divmod(a, b)
    return {a // b, a % b}
end

-- Test functions
function test_tail_recursion()
    if sum_range(100, 0) ~= 5050 then return 0 end
    return 1
end

function test_local_function()
    if fact(0) ~= 1 then return 0 end
    if fact(1) ~= 1 then return 0 end
    if fact(5) ~= 120 then return 0 end
    if fact(10) ~= 3628800 then return 0 end
    return 1
end

function test_multi_params()
    if add8(1, 2, 3, 4, 5, 6, 7, 8) ~= 36 then return 0 end
    return 1
end

function test_fibonacci()
    if fib(0) ~= 0 then return 0 end
    if fib(1) ~= 1 then return 0 end
    if fib(10) ~= 55 then return 0 end
    if fib(20) ~= 6765 then return 0 end
    return 1
end

function test_gcd_func()
    if gcd_func(12, 8) ~= 4 then return 0 end
    if gcd_func(100, 75) ~= 25 then return 0 end
    if gcd_func(17, 13) ~= 1 then return 0 end
    if gcd_func(0, 5) ~= 5 then return 0 end
    return 1
end

function test_multi_return_via_table()
    local r = divmod(17, 5)
    if r[1] ~= 3 then return 0 end
    if r[2] ~= 2 then return 0 end
    return 1
end

function test_recursive_power()
    if power(2, 10) ~= 1024 then return 0 end
    if power(3, 5) ~= 243 then return 0 end
    if power(5, 0) ~= 1 then return 0 end
    return 1
end
