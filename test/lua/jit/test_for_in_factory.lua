-- Generic for-in explist adjustment (Lua 5.4):
--   1 exp -> f, s, var from that exp's returns (missing become nil)
--   2 exp -> f, plus s, var from last exp
--   3 exp -> f, s, var; extras discarded
-- Avoid native ipairs/next as values (call-by-name only, not first-class).

local function iter(t, i)
    i = i + 1
    local v = t[i]
    if v == nil then
        return nil
    end
    return i, v
end

function test_for_in_factory()
    local function factory()
        return iter, {10, 20, 30}, 0
    end
    local s = 0
    for i, v in factory() do
        s = s + v
    end
    if s ~= 60 then
        return 0
    end

    local function factory_one()
        return factory()
    end
    s = 0
    for i, v in factory_one() do
        s = s + v
    end
    if s ~= 60 then
        return 0
    end
    return 1
end

function test_for_in_mypairs()
    local function mypairs(t)
        return iter, t, 0
    end
    local s = 0
    for i, v in mypairs({10, 20, 30}) do
        s = s + v
    end
    if s ~= 60 then
        return 0
    end
    return 1
end

function test_for_in_two_returns()
    local function give_state()
        return {7}, 0
    end
    local s = 0
    for k, v in iter, give_state() do
        s = s + v
    end
    if s ~= 7 then
        return 0
    end

    s = 0
    local t = {4, 5}
    local ctrl = 0
    for k, v in iter, t, ctrl do
        s = s + v
    end
    if s ~= 9 then
        return 0
    end
    return 1
end
