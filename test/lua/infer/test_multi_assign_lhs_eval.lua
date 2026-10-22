-- Multi-assignment must evaluate all RHS and all LHS table base/key
-- expressions before any stores (Lua assignment semantics).

-- Classic: i, t[i] = ... must index with the old i.
function test_index_before_store()
    local t = {0, 0, 0}
    local i = 1
    i, t[i] = 2, 99
    if i ~= 2 then return 0 end
    if t[1] ~= 99 then return 0 end
    if t[2] ~= 0 then return 0 end
    return 1
end

-- t, t.x = {}, 1 writes the field on the old table.
function test_dot_base_before_store()
    local t = {x = 0}
    local old = t
    t, t.x = {}, 1
    if old.x ~= 1 then return 0 end
    if t.x ~= nil then return 0 end
    return 1
end

-- Side-effecting key is evaluated once, before stores.
function test_side_effect_key()
    local n = 0
    local function bump()
        n = n + 1
        return n
    end
    local t = {}
    local i = 1
    i, t[bump()] = 2, 10
    if i ~= 2 then return 0 end
    if t[1] ~= 10 then return 0 end
    if n ~= 1 then return 0 end
    return 1
end

-- Indexed LHS expressions are evaluated before RHS expressions.
function test_lhs_before_rhs_order()
    local phase = 0
    local t = {}
    local x = 0
    local function key()
        if phase ~= 0 then return 99 end
        phase = 1
        return 1
    end
    local function rhs()
        if phase ~= 1 then return -1 end
        phase = 2
        return 42
    end
    t[key()], x = rhs(), 7
    if phase ~= 2 or t[1] ~= 42 or x ~= 7 then return 0 end
    return 1
end

-- Simple locals still swap correctly.
function test_simple_swap()
    local a, b = 1, 2
    a, b = b, a
    if a ~= 2 then return 0 end
    if b ~= 1 then return 0 end
    return 1
end
