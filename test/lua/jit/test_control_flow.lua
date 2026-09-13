-- Lua 5.4 control flow comprehensive tests

function classify(i)
    if i < 10 then
        return "small"
    elseif i < 20 then
        return "medium"
    elseif i < 30 then
        return "large"
    else
        return "huge"
    end
end

function test_nested_for()
    local n = 100
    local a = 0
    local t = {}
    for i = 1, n do
        for j = i, 1, -1 do
            a = a + 1
            t[j] = 1
        end
    end
    if a ~= n * (n + 1) / 2 then return 0 end
    if not t[1] or not t[n] or t[0] or t[n + 1] then return 0 end
    return 1
end

function test_repeat_break()
    local x = 1
    local result = 0
    repeat
        if x == 5 then
            result = 100
            break
        end
        x = x + 1
    until x >= 10
    if result ~= 100 then return 0 end
    if x ~= 5 then return 0 end
    return 1
end

function test_while_complex()
    local a = 1
    local b = 1
    for i = 1, 10 do
        local temp = a + b
        a = b
        b = temp
    end
    if a ~= 89 then return 0 end
    if b ~= 144 then return 0 end
    return 1
end

function test_if_elseif_chain()
    if classify(3) ~= "small" then return 0 end
    if classify(12) ~= "medium" then return 0 end
    if classify(26) ~= "large" then return 0 end
    if classify(100) ~= "huge" then return 0 end
    return 1
end

function test_break_nested()
    local found = 0
    for i = 1, 10 do
        for j = 1, 10 do
            if i * j == 42 then
                found = i * 100 + j
                break
            end
        end
        if found > 0 then break end
    end
    if found ~= 607 then return 0 end
    return 1
end

function test_repeat_complex()
    local x = 1
    local result = 0
    repeat
        result = result + x
        x = x + 1
    until x > 10
    if result ~= 55 then return 0 end
    return 1
end

function test_for_step()
    local sum = 0
    for i = 10, 1, -1 do
        sum = sum + i
    end
    if sum ~= 55 then return 0 end
    local sum2 = 0
    for i = 0, 20, 3 do
        sum2 = sum2 + i
    end
    if sum2 ~= 63 then return 0 end
    return 1
end

function test_for_break()
    local sum = 0
    for i = 1, 100 do
        if i > 10 then break end
        sum = sum + i
    end
    if sum ~= 55 then return 0 end
    return 1
end

function test_deep_if()
    local a, b, c, d = 1, 2, 3, 4
    if a > 0 then
        if b > 0 then
            if c > 0 then
                if d > 0 then
                    return 1
                end
            end
        end
    end
    return 0
end
