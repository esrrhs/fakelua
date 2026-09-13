-- Lua 5.4 edge cases and type inference stress tests

function max_val(a, b)
    return a > b and a or b
end

function min_val(a, b)
    return a < b and a or b
end

function check_logical(a, b, c, d, e)
    if not (a >= b or c or d and e or nil) then
        return 0
    else
        return 1
    end
end

function sum_digits(n)
    local s = 0
    while n > 0 do
        s = s + n % 10
        n = n // 10
    end
    return s
end

function reverse_number(n)
    local r = 0
    while n > 0 do
        r = r * 10 + n % 10
        n = n // 10
    end
    return r
end

function test_mixed_arithmetic()
    local a = 10
    if a + 5 ~= 15 then return 0 end
    if a - 3 ~= 7 then return 0 end
    if a * 2 ~= 20 then return 0 end
    if a // 3 ~= 3 then return 0 end
    if a % 3 ~= 1 then return 0 end
    return 1
end

function test_integer_wrap()
    local a = 9223372036854775807
    local b = a + 1
    if b ~= -9223372036854775808 then return 0 end
    return 1
end

function test_chained_comparison()
    local a, b, c = 1, 2, 3
    if not (a < b and b < c) then return 0 end
    if not (a <= b and b <= c and a <= c) then return 0 end
    if not (c > b and b > a) then return 0 end
    if not (c >= b and b >= a) then return 0 end
    return 1
end

function test_complex_logical()
    if check_logical(2, 1, nil, nil, nil) ~= 1 then return 0 end
    if check_logical(1, 2, 1, nil, nil) ~= 1 then return 0 end
    if check_logical(1, 2, nil, nil, 1) ~= 0 then return 0 end
    if check_logical(1, 2, nil, 1, nil) ~= 0 then return 0 end
    return 1
end

function test_local_scope()
    local x = 10
    do
        local x = 20
        if x ~= 20 then return 0 end
    end
    if x ~= 10 then return 0 end
    return 1
end

function test_for_edge()
    local sum = 0
    for i = 10, 1 do
        sum = sum + 1
    end
    if sum ~= 0 then return 0 end
    for i = 5, 5 do
        sum = sum + 1
    end
    if sum ~= 1 then return 0 end
    local t = {}
    for i = 5, 1, -1 do
        t[#t + 1] = i
    end
    if #t ~= 5 then return 0 end
    if t[1] ~= 5 then return 0 end
    if t[5] ~= 1 then return 0 end
    return 1
end

function test_string_basic()
    local s = "hello" .. " " .. "world"
    if #s ~= 11 then return 0 end
    if s ~= "hello world" then return 0 end
    return 1
end

function test_ternary_pattern()
    if max_val(3, 5) ~= 5 then return 0 end
    if max_val(7, 2) ~= 7 then return 0 end
    if min_val(3, 5) ~= 3 then return 0 end
    if min_val(7, 2) ~= 2 then return 0 end
    return 1
end

function test_destructuring()
    local a, b = 1, 2
    a, b = b, a
    if a ~= 2 or b ~= 1 then return 0 end
    return 1
end

function test_operator_stress()
    local a, b, c = 2, 3, 4
    if a + b * c ~= 14 then return 0 end
    if (a + b) * c ~= 20 then return 0 end
    if a * b + c ~= 10 then return 0 end
    if a ^ b ^ c ~= a ^ (b ^ c) then return 0 end
    if -a * b ~= -6 then return 0 end
    if a * -b ~= -6 then return 0 end
    if not (a + b > c) then return 0 end
    if not (not (a + b < c)) then return 0 end
    if not (a < b and b < c) then return 0 end
    if not (a <= b or b > c) then return 0 end
    return 1
end

function test_sum_digits_func()
    if sum_digits(123) ~= 6 then return 0 end
    if sum_digits(9999) ~= 36 then return 0 end
    if sum_digits(0) ~= 0 then return 0 end
    return 1
end

function test_reverse_number_func()
    if reverse_number(123) ~= 321 then return 0 end
    if reverse_number(100) ~= 1 then return 0 end
    if reverse_number(12345) ~= 54321 then return 0 end
    return 1
end
