package "ContainerTest"

function test_deque()
    local d = container.deque()
    if not d:empty() then return 0 end
    d:push_back(1)
    d:push_back("b")
    d:push_front(0)
    if d:size() ~= 3 then return 0 end
    if d:front() ~= 0 then return 0 end
    if d:back() ~= "b" then return 0 end
    if d:at(2) ~= 1 then return 0 end
    d:set(2, 9)
    if d:at(2) ~= 9 then return 0 end
    if d:pop_front() ~= 0 then return 0 end
    if d:pop_back() ~= "b" then return 0 end
    if d:size() ~= 1 then return 0 end
    local t = d:to_table()
    if t[1] ~= 9 then return 0 end
    d:clear()
    if not d:empty() then return 0 end
    if d:pop_front() ~= nil then return 0 end
    if d:at(1) ~= nil then return 0 end
    return 1
end

function test_map()
    local m = container.map()
    m:set("b", 2)
    m:set("a", 1)
    m:set(3, "three")
    if m:size() ~= 3 then return 0 end
    if not m:has("a") then return 0 end
    if m:get("a") ~= 1 then return 0 end
    if m:get("missing") ~= nil then return 0 end
    m:set("a", 10)
    if m:get("a") ~= 10 then return 0 end
    if not m:erase("b") then return 0 end
    if m:has("b") then return 0 end
    local keys = m:keys()
    if #keys ~= 2 then return 0 end
    if keys[1] ~= 3 then return 0 end
    if keys[2] ~= "a" then return 0 end
    local tbl = m:to_table()
    if tbl["a"] ~= 10 then return 0 end
    if tbl[3] ~= "three" then return 0 end
    if m:erase("missing") then return 0 end
    m:set(true, "yes")
    if m:get(true) ~= "yes" then return 0 end
    m:clear()
    if not m:empty() then return 0 end
    if m:size() ~= 0 then return 0 end
    return 1
end

function test_set()
    local s = container.set()
    if not s:insert(2) then return 0 end
    if not s:insert(1) then return 0 end
    if s:insert(2) then return 0 end
    if s:size() ~= 2 then return 0 end
    if not s:has(1) then return 0 end
    if s:has(9) then return 0 end
    local v = s:values()
    if #v ~= 2 or v[1] ~= 1 or v[2] ~= 2 then return 0 end
    if not s:erase(1) then return 0 end
    if s:has(1) then return 0 end
    if s:erase(1) then return 0 end
    s:clear()
    if not s:empty() then return 0 end
    if not s:insert("z") then return 0 end
    if not s:insert("a") then return 0 end
    local v2 = s:values()
    if #v2 ~= 2 or v2[1] ~= "a" or v2[2] ~= "z" then return 0 end
    return 1
end

function test_closed()
    local d = container.deque()
    d:close()
    d:size()
    return 0
end

function test_bad_table_value()
    local d = container.deque()
    local t = {}
    t[1] = 1
    d:push_back(t)
    return 0
end

function test_deque_set_oor()
    local d = container.deque()
    d:push_back(1)
    d:set(2, 9)
    return 0
end

function test_vector()
    local v = container.vector()
    if not v:empty() then return 0 end
    v:push_back(1)
    v:push_back("b")
    v:push_back(3)
    if v:size() ~= 3 then return 0 end
    if v:front() ~= 1 then return 0 end
    if v:back() ~= 3 then return 0 end
    if v:at(2) ~= "b" then return 0 end
    if v:at(0) ~= nil then return 0 end
    if v:at(9) ~= nil then return 0 end
    v:set(2, "x")
    if v:at(2) ~= "x" then return 0 end
    if v:pop_back() ~= 3 then return 0 end
    if v:size() ~= 2 then return 0 end
    local t = v:to_table()
    if #t ~= 2 or t[1] ~= 1 or t[2] ~= "x" then return 0 end
    v:clear()
    if not v:empty() then return 0 end
    if v:pop_back() ~= nil then return 0 end
    if v:front() ~= nil then return 0 end
    if v:back() ~= nil then return 0 end
    return 1
end

function test_small_vector()
    local v = container.small_vector()
    if not v:empty() then return 0 end
    local i = 1
    while i <= 12 do
        v:push_back(i)
        i = i + 1
    end
    if v:size() ~= 12 then return 0 end
    if v:front() ~= 1 then return 0 end
    if v:back() ~= 12 then return 0 end
    if v:at(8) ~= 8 then return 0 end
    if v:at(9) ~= 9 then return 0 end
    v:set(1, 100)
    if v:at(1) ~= 100 then return 0 end
    if v:pop_back() ~= 12 then return 0 end
    local t = v:to_table()
    if #t ~= 11 or t[1] ~= 100 or t[11] ~= 11 then return 0 end
    v:clear()
    if not v:empty() then return 0 end
    return 1
end

function test_list()
    local l = container.list()
    if not l:empty() then return 0 end
    l:push_back(2)
    l:push_back(3)
    l:push_front(1)
    if l:size() ~= 3 then return 0 end
    if l:front() ~= 1 then return 0 end
    if l:back() ~= 3 then return 0 end
    if l:at(2) ~= 2 then return 0 end
    l:set(2, 20)
    if l:at(2) ~= 20 then return 0 end
    if l:pop_front() ~= 1 then return 0 end
    if l:pop_back() ~= 3 then return 0 end
    if l:size() ~= 1 then return 0 end
    local t = l:to_table()
    if t[1] ~= 20 then return 0 end
    l:clear()
    if not l:empty() then return 0 end
    if l:pop_front() ~= nil then return 0 end
    if l:at(1) ~= nil then return 0 end
    return 1
end

function test_nested()
    local inner = container.vector()
    inner:push_back(7)
    local d = container.deque()
    d:push_back(inner)
    local got = d:at(1)
    if got:at(1) ~= 7 then return 0 end
    return 1
end

function test_vector_set_oor()
    local v = container.vector()
    v:push_back(1)
    v:set(2, 9)
    return 0
end

function test_list_set_oor()
    local l = container.list()
    l:push_back(1)
    l:set(0, 9)
    return 0
end

function test_bad_fn_value()
    local v = container.vector()
    v:push_back(function() end)
    return 0
end
