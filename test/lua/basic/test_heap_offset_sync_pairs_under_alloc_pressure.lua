-- Regression for FlFastAlloc / HeapAllocator offset desync.
-- JIT table/string construction uses FlFastAlloc (bumps slot.curr only).
-- next/pairs go through C++ ForEachKV and may Alloc on the same heap.
-- Without offset sync those paths overlap → wrong next counts / SEGV.

function test_heap_offset_sync_pairs_under_alloc_pressure()
    local acc = 0
    for round = 1, 64 do
        local t = {}
        -- Mix string keys (FlFastAlloc for dynamic strings) and inserts that may grow via C++ Alloc.
        for i = 1, 20 do
            local key = "k" .. tostring(i) .. "_" .. tostring(round)
            t[key] = i * round
            t[i] = i + round
        end

        local count = 0
        local sum = 0
        local seen = {}
        for k, v in pairs(t) do
            if seen[k] then return 1 end
            seen[k] = true
            count = count + 1
            sum = sum + v
        end
        -- 20 string keys + 20 integer keys
        if count ~= 40 then return 2 end
        acc = acc + sum
    end
    if acc <= 0 then return 3 end
    return 5000
end
