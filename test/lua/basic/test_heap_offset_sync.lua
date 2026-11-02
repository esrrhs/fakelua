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
