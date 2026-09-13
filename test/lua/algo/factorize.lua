-- Is prime check
function is_prime(n)
    if n < 2 then return 0 end
    local d = 2
    while d * d <= n do
        if n % d == 0 then return 0 end
        d = d + 1
    end
    return 1
end

-- Count primes up to n
function count_primes(n)
    local count = 0
    for i = 2, n do
        if is_prime(i) == 1 then count = count + 1 end
    end
    return count
end
