-- Regression for FlFastAlloc / HeapAllocator offset desync.
-- JIT table/string construction uses FlFastAlloc (bumps slot.curr only).
-- next/pairs go through C++ ForEachKV and may Alloc on the same heap.
-- Without offset sync those paths overlap → wrong next counts / SEGV.

function test_heap_offset_sync_nested_tables_next()
    local root = {}
    for i = 1, 64 do
        root[i] = {
            id = i,
            child = { x = i, y = i * 2, z = i * 3 },
            tag = "n" .. tostring(i),
        }
    end

    for i = 1, 64 do
        local node = root[i]
        local ncount = 0
        local nk = nil
        while true do
            local key = next(node, nk)
            if key == nil then break end
            ncount = ncount + 1
            nk = key
        end
        if ncount ~= 3 then return 10 + i end

        local child = node.child
        local ccount = 0
        local ck = nil
        local csum = 0
        while true do
            local key, val = next(child, ck)
            if key == nil then break end
            ccount = ccount + 1
            if type(val) == "number" then csum = csum + val end
            ck = key
        end
        if ccount ~= 3 then return 100 + i end
        if csum ~= i + i * 2 + i * 3 then return 200 + i end
        if node.id ~= i then return 300 + i end
    end
    return 5000
end
