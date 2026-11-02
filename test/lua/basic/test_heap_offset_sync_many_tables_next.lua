-- Regression for FlFastAlloc / HeapAllocator offset desync.
-- JIT table/string construction uses FlFastAlloc (bumps slot.curr only).
-- next/pairs go through C++ ForEachKV and may Alloc on the same heap.
-- Without offset sync those paths overlap → wrong next counts / SEGV.

function test_heap_offset_sync_many_tables_next()
    local tables = {}
    for i = 1, 256 do
        -- Spec-key table constructors allocate VarTable + spec arrays via FlFastAlloc.
        tables[i] = { a = i, b = i + 1, c = i + 2 }
    end

    for i = 1, 256 do
        local t = tables[i]
        local count = 0
        local k = nil
        local seen = {}
        while true do
            local key, val = next(t, k)
            if key == nil then break end
            if seen[key] then return 10 + i end
            seen[key] = true
            if t[key] ~= val then return 300 + i end
            count = count + 1
            k = key
        end
        if count ~= 3 then return i end
        if t.a ~= i or t.b ~= i + 1 or t.c ~= i + 2 then return 600 + i end
    end
    return 5000
end
