-- Interpreter error and edge paths (len/index/for-step/bitwise/_VERSION).
function test_interp_errors()
    local ok, err = pcall(function() return #123 end)
    if ok then return 1 end
    ok, err = pcall(function()
        local t = 1
        return t[1]
    end)
    if ok then return 2 end
    ok, err = pcall(function()
        local t = {}
        return t[nil]
    end)
    if ok then return 3 end
    local function zero_step()
        return 0
    end
    ok, err = pcall(function()
        for i = 1, 10, zero_step() do
            return i
        end
    end)
    if ok then return 4 end
    local function zero_fstep()
        return 0.0
    end
    ok, err = pcall(function()
        for i = 1.0, 10.0, zero_fstep() do
            return i
        end
    end)
    if ok then return 5 end
    -- _VERSION global
    if type(_VERSION) ~= "string" then return 6 end
    local function nested_local_fn()
        return 7
    end
    if nested_local_fn() ~= 7 then return 8 end
    -- for-limit edge: huge float limit with negative step should skip
    local n = 0
    for i = 1, 1e308, -1 do
        n = n + 1
        if n > 2 then break end
    end
    for i = 1, -1e308, 1 do
        n = n + 100
    end
    -- bitwise on non-number
    ok = pcall(function() return 1 & "x" end)
    if ok then return 9 end
    return 0
end
