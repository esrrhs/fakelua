function isprime(n)
    local nn = n - 1
    for i = 2, nn do
        if (n % i == 0) then
            return false
        end
    end
    return true
end


function primes(n)
    local count = 0

    for i = 2, n do
        if (isprime(i)) then
            count = count + 1
        end
    end
    return count
end

local N = 10 * 10000

print(primes(N))
