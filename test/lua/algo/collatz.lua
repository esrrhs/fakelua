-- Collatz sequence length
function collatz_len(n)
    local len = 1
    while n ~= 1 do
        if n % 2 == 0 then n = n // 2 else n = 3 * n + 1 end
        len = len + 1
    end
    return len
end

-- Find max Collatz length under limit
function collatz_max_len(limit)
    local max_len = 0
    local max_num = 0
    for i = 1, limit do
        local len = collatz_len(i)
        if len > max_len then max_len = len; max_num = i end
    end
    return max_num
end
