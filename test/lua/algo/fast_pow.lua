-- Fast modular exponentiation: (base ^ exp) % mod using repeated squaring.
-- Exercises: while loop, bitwise AND (&), right shift (>>), integer arithmetic.
function test(base, exp, mod)
    local result = 1
    base = base % mod
    while exp > 0 do
        if (exp & 1) == 1 then
            result = (result * base) % mod
        end
        exp = exp >> 1
        base = (base * base) % mod
    end
    return result
end

-- Check if a number is a perfect power of two using bitwise ops.
-- Returns 1 (true) or 0 (false).
-- Exercises: bitwise AND, comparison with 0, if/else.
function test_is_pow2(n)
    if n <= 0 then
        return 0
    end
    if (n & (n - 1)) == 0 then
        return 1
    end
    return 0
end

-- Count set bits (popcount) via Brian Kernighan's algorithm.
-- Exercises: while, bitwise AND, subtraction, counter.
function test_popcount(n)
    local count = 0
    while n ~= 0 do
        n = n & (n - 1)
        count = count + 1
    end
    return count
end
