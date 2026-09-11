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
