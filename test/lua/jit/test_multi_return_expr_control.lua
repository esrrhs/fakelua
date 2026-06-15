local function get_cond_bool()
    return true, false
end

local function get_start_val()
    return 1, 10
end

local function get_limit_val()
    return 5, 20
end

function test_if()
    if get_cond_bool() then
        return 1
    else
        return 0
    end
end

function test_while()
    local i = 0
    while get_cond_bool() do
        i = i + 1
        if i >= 3 then
            break
        end
    end
    return i
end

function test_for()
    local sum = 0
    for i = get_start_val(), get_limit_val() do
        sum = sum + i
    end
    return sum
end
