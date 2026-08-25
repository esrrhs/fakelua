-- Test pcall basic usage: success case, error case, multi-return
function test_pcall_success()
    local ok, val = pcall(function() return 42 end)
    if not ok then return 1 end
    if val ~= 42 then return 2 end
    return 5000
end

function test_pcall_error()
    local ok, msg = pcall(function() error("boom") end)
    if ok then return 1 end
    if type(msg) ~= "string" then return 2 end
    if string.find(msg, "boom") == nil then return 3 end
    return 5000
end

function test_pcall_multi_return()
    local ok, a, b, c = pcall(function() return 1, 2, 3 end)
    if not ok then return 1 end
    if a ~= 1 or b ~= 2 or c ~= 3 then return 2 end
    return 5000
end

function test_pcall_with_args()
    local ok, result = pcall(function(x, y) return x + y end, 10, 20)
    if not ok then return 1 end
    if result ~= 30 then return 2 end
    return 5000
end

function test_pcall_non_function()
    local ok, msg = pcall(42)
    if ok then return 1 end
    if type(msg) ~= "string" then return 2 end
    return 5000
end

-- DispatchCall 必须把闭包指针传给 JIT 函数，否则捕获的 upvalue 会空指针崩溃。
function test_pcall_upvalue()
    local x = 7
    local ok, r = pcall(function() return x + 1 end)
    if not ok then return 1 end
    if r ~= 8 then return 2 end
    return 5000
end

function test_xpcall_success()
    local handler_called = false
    local function handler(err)
        handler_called = true
        return "handled: " .. err
    end
    local ok, val = xpcall(function() return 99 end, handler)
    if not ok then return 1 end
    if val ~= 99 then return 2 end
    if handler_called then return 3 end
    return 5000
end

function test_xpcall_error()
    local function handler(err)
        return "handled: " .. err
    end
    local ok, val = xpcall(function() error("fail") end, handler)
    if ok then return 1 end
    if type(val) ~= "string" then return 2 end
    return 5000
end

function test_xpcall_non_function()
    local function handler(err)
        return "err: " .. err
    end
    local ok, msg = xpcall(123, handler)
    if ok then return 1 end
    return 5000
end

-- 以前 xpcall 只拷前 16 个实参，第 17 个会被丢掉
function test_xpcall_many_args()
    local function take17(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17)
        return a17
    end
    local ok, val = xpcall(take17, function(err) return err end,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17)
    if not ok then return 1 end
    if val ~= 17 then return 2 end
    return 5000
end

-- 缺参必须补 nil 再调 JIT，否则按实参个数选错函数指针（UB）
function test_pcall_missing_args()
    local function add(a, b)
        if a == nil then return 11 end
        if b == nil then return a + 20 end
        return a + b
    end
    local ok1, r1 = pcall(add)
    if not ok1 then return 1 end
    if r1 ~= 11 then return 2 end
    local ok2, r2 = pcall(add, 5)
    if not ok2 then return 3 end
    if r2 ~= 25 then return 4 end
    local ok3, r3 = pcall(add, 5, 7)
    if not ok3 then return 5 end
    if r3 ~= 12 then return 6 end
    return 5000
end
