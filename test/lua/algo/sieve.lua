-- Sieve of Eratosthenes: count primes <= n.
-- Exercises: table construction with boolean values, nested while loops,
-- for-loop with step, modulo (%), multiplication, comparison.
function test(n)
    local is_prime = {}
    for i = 2, n do
        is_prime[i] = true
    end

    local i = 2
    while i * i <= n do
        if is_prime[i] then
            local j = i * i
            while j <= n do
                is_prime[j] = false
                j = j + i
            end
        end
        i = i + 1
    end

    local count = 0
    for k = 2, n do
        if is_prime[k] then
            count = count + 1
        end
    end
    return count
end

-- Return the n-th prime (1-indexed).
-- Exercises: repeat-until, combined loop control.
function test_nth_prime(n)
    local count = 0
    local candidate = 1
    repeat
        candidate = candidate + 1
        local is_p = true
        local d = 2
        while d * d <= candidate do
            if candidate % d == 0 then
                is_p = false
            end
            d = d + 1
        end
        if is_p then
            count = count + 1
        end
    until count == n
    return candidate
end
