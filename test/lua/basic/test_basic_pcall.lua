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
