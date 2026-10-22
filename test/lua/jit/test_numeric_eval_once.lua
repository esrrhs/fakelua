function test_precise_mixed_compare()
    local i = (1 << 53) + 1
    local f = (1 << 53) + 0.0
    if i == f then return 1 end
    if i <= f then return 2 end
    if not (i > f) then return 3 end
    if f >= i then return 4 end

    local t = {}
    t[i] = 11
    t[f] = 22
    if t[i] ~= 11 or t[f] ~= 22 then return 5 end

    local a = {i, f}
    table.sort(a)
    if a[1] ~= f or a[2] ~= i then return 6 end
    return 0
end

function test_and_or_eval_once()
    local count = 0
    local t = {false}
    local function key()
        count = count + 1
        return 1
    end
    local a = t[key()] and 7
    local b = t[key()] or 8
    if a ~= false or b ~= 8 then return -1 end
    return count
end

function test_math_inline_eval_once()
    local count = 0
    local t = {3.75, 8.25, -2, 5, 7}
    local function key()
        count = count + 1
        return count
    end
    local a = math.floor(t[key()])
    local b = math.ceil(t[key()])
    local c = math.abs(t[key()])
    local d = math.max(t[key()], 1)
    local e = math.min(t[key()], 10)
    if a ~= 3 or b ~= 9 or c ~= 2 or d ~= 5 or e ~= 7 then
        return -1
    end
    return count
end

function bump(counter, x)
    counter.n = counter.n + 1
    return x
end

function test_numeric_ternary_once(n)
    local counter = {n = 0}
    local result = (n > 0 and bump(counter, 1)) or bump(counter, 2)
    if (n > 0 and result ~= 1) or (n <= 0 and result ~= 2) then
        return -1
    end
    return counter.n
end
