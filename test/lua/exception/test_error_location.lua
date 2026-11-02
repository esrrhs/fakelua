package "ErrorLocation"

-- 运行期错误必须带 file:line。每个函数的出错行都写死在下面的断言里。
function arith_on_nil()
    local t = {}
    return t.missing + 1
end

function index_non_table()
    local a = nil
    return a.b.c
end

function call_non_function()
    local f = nil
    f()
    return 1
end

-- 原生函数抛的错也要定位到调用它的那条 Lua 语句
function native_bad_argument()
    return string.rep("x", "not a number")
end

-- 位置要取最内层那条语句，不是主叫方
local function innermost(x)
    return x.missing
end

function outer_call()
    return innermost(nil)
end

function nested_frames()
    local ok, err = pcall(outer_call)
    if ok then return 0 end
    if string.find(err, "test_error_location.lua:27") == nil then return 0 end
    return 1
end

local function returns_nil()
    return nil
end

function error_after_callee_returns()
    local x = returns_nil() + 1
    return x
end

function make_throwing_closure()
    return function(x)
        return x.missing + 1
    end
end

function make_returning_closure()
    return function()
        return nil
    end
end
