-- Regression for FlFastAlloc / HeapAllocator offset desync.
-- JIT table/string construction uses FlFastAlloc (bumps slot.curr only).
-- next/pairs go through C++ ForEachKV and may Alloc on the same heap.
-- Without offset sync those paths overlap → wrong next counts / SEGV.

function test_heap_offset_sync_string_keys_and_rehash()
    -- Build past the quick/spec threshold so rehash uses C++ Alloc while
    -- prior table/string pieces came from FlFastAlloc.
    local t = { a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8 }
    for i = 1, 40 do
        t["dyn_" .. tostring(i)] = i
    end

    local count = 0
    local seen = {}
    for k, v in pairs(t) do
        if seen[k] then return 1 end
        seen[k] = true
        count = count + 1
    end
    if count ~= 48 then return 2 end

    -- next traversal must match pairs count and preserve values.
    local ncount = 0
    local k = nil
    while true do
        local key, val = next(t, k)
        if key == nil then break end
        if t[key] ~= val then return 3 end
        ncount = ncount + 1
        k = key
    end
    if ncount ~= 48 then return 4 end
    return 5000
end
