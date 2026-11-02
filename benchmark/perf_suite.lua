package "main"

local function fib(n)
    if n <= 1 then return n end
    return fib(n - 1) + fib(n - 2)
end

local function bench_fib()
    local r = 0
    for i = 1, 5 do
        r = fib(28)
    end
    return r
end

local function bench_sum()
    local s = 0
    for i = 1, 10000000 do
        s = s + i
    end
    return s
end

local function bench_sieve()
    local n = 50000
    local is_prime = {}
    for i = 2, n do is_prime[i] = true end
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
        if is_prime[k] then count = count + 1 end
    end
    return count
end

local function bench_bubble_sort()
    local n = 300
    local t = {}
    for i = 1, n do t[i] = n - i end
    for i = 1, n do
        for j = 1, n - i do
            if t[j] > t[j + 1] then
                local tmp = t[j]
                t[j] = t[j + 1]
                t[j + 1] = tmp
            end
        end
    end
    return t[1]
end

local function bench_hash_map()
    local t = {}
    for i = 1, 5000 do
        t["k_" .. tostring(i)] = i
    end
    local s = 0
    for i = 1, 5000 do
        s = s + (t["k_" .. tostring(i)] or 0)
    end
    return s
end

local function make_counter(start)
    local c = start
    return function()
        c = c + 1
        return c
    end
end

local function bench_closure()
    local s = 0
    for i = 1, 100000 do
        local c1 = make_counter(i)
        s = s + c1() + c1()
    end
    return s
end

local function bench_matmul()
    local n = 60
    local a, b, c = {}, {}, {}
    for i = 1, n * n do
        a[i] = i % 10
        b[i] = (i * 3) % 10
        c[i] = 0
    end
    for i = 0, n - 1 do
        for j = 0, n - 1 do
            local sum = 0
            for k = 0, n - 1 do
                sum = sum + a[i * n + k + 1] * b[k * n + j + 1]
            end
            c[i * n + j + 1] = sum
        end
    end
    return c[1]
end

local function run_one(name, fn)
    local t0 = os.clock()
    local res = fn()
    local elapsed = (os.clock() - t0) * 1000
    print(string.format("%-18s: %10.3f ms  (result: %s)", name, elapsed, tostring(res)))
    return elapsed
end

function init()
    print("=== FakeLua Benchmark Suite ===")
    run_one("fib(28) x5", bench_fib)
    run_one("sum(1e7)", bench_sum)
    run_one("sieve(5e4)", bench_sieve)
    run_one("bubble_sort(300)", bench_bubble_sort)
    run_one("matmul(60x60)", bench_matmul)
    run_one("closure(1e5)", bench_closure)
    run_one("hash_map(5e3)", bench_hash_map)
    print("================================")
    return 0
end

function fini()
    return 0
end


