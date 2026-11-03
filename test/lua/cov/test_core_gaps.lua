-- Exercise less-covered core compile/runtime paths.
function test_string_escapes()
    local s = "a\001b\n\tc\127d"
    if #s < 5 then return 1 end
    if string.byte(s, 2) ~= 1 then return 2 end
    return 0
end

function test_math_inline_extra()
    if math.abs(math.fmod(10, 3) - 1) > 1e-9 then return 1 end
    if math.abs(math.ldexp(2.0, 3) - 16.0) > 1e-9 then return 2 end
    if math.type(7) ~= "integer" then return 3 end
    if math.type(7.5) ~= "float" then return 4 end
    if math.type("x") ~= nil then return 5 end
    if math.tointeger(9.0) ~= 9 then return 6 end
    if math.tointeger(9.2) ~= nil then return 7 end
    if not math.ult(1, 2) then return 8 end
    if math.ult(-1, 1) then return 9 end
    local ok = pcall(function() return math.fmod("a", 1) end)
    if ok then return 10 end
    -- string -> tointeger native fallback
    if math.tointeger("11") ~= 11 then return 11 end
    return 0
end

function returns_two()
    return 10, 20
end

function test_local_multi_assign()
    local a, b, c = 1, returns_two()
    if a ~= 1 or b ~= 10 or c ~= 20 then return 1 end
    local x, y = returns_two()
    if x ~= 10 or y ~= 20 then return 2 end
    return 0
end

function test_table_seq_and_move()
    local t = {10, 20, 30, 40, 50}
    if #t ~= 5 then return 1 end
    -- force hash path mixed with array
    t["k"] = "v"
    if t[1] ~= 10 or t.k ~= "v" then return 2 end
    -- table.move / insert / remove paths
    table.insert(t, 2, 15)
    if t[2] ~= 15 then return 3 end
    local r = table.remove(t, 2)
    if r ~= 15 then return 4 end
    local dst = {}
    table.move(t, 1, 3, 1, dst)
    if dst[1] ~= 10 or dst[3] ~= 30 then return 5 end
    -- sparse then fill for seq scan
    local s = {}
    s[1] = 1
    s[3] = 3
    s[2] = 2
    if #s < 2 then return 6 end
    return 0
end

function test_string_ops_extra()
    if string.lower("AbC") ~= "abc" then return 1 end
    if string.upper("AbC") ~= "ABC" then return 2 end
    if string.reverse("ab") ~= "ba" then return 3 end
    if string.rep("a", 3) ~= "aaa" then return 4 end
    if string.byte("ABC", 2) ~= 66 then return 5 end
    if string.char(65) ~= "A" then return 6 end
    if string.find("hello", "ll", 1, true) ~= 3 then return 7 end
    if tonumber("3.5") ~= 3.5 then return 8 end
    return 0
end

function many_args(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10)
    return a1+a2+a3+a4+a5+a6+a7+a8+a9+a10
end

function test_select_and_closure_args()
    if select("#", 1, 2, 3) ~= 3 then return 1 end
    if select(2, 10, 20, 30) ~= 20 then return 2 end
    local f = function(...)
        return select("#", ...)
    end
    if f(1, 2, 3, 4) ~= 4 then return 3 end
    if many_args(1,2,3,4,5,6,7,8,9,10) ~= 55 then return 4 end
    -- closure call with multi returns flattened
    local function ret_multi() return 1, 2, 3 end
    local function take3(a, b, c) return a + b + c end
    if take3(ret_multi()) ~= 6 then return 5 end
    return 0
end

-- File-level locals for LiteralFromExp float/unop paths in interp codegen.
local cov_neg = -42
local cov_flt = 1.25
local cov_paren = (-3.5)

function test_file_level_literals()
    if cov_neg ~= -42 then return 1 end
    if cov_flt ~= 1.25 then return 2 end
    if cov_paren ~= -3.5 then return 3 end
    return 0
end

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

function test_empty_body()
    return 0
end

function test_bool_nil_locals()
    local a = nil
    local b = true
    local c = false
    if a ~= nil then return 1 end
    if b ~= true then return 2 end
    if c ~= false then return 3 end
    return 0
end
