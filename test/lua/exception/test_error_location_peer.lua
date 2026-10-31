package "ErrorLocationPeer"

-- 跨编译单元（多个 .so）调用测试：本文件在 test_error_location.lua 之后加载。
-- 1) 调用前一个 .so 返回的闭包，闭包内部在第 52 行抛错，位置必须指向 test_error_location.lua:52。
function call_peer_throwing_closure()
    local fn = ErrorLocation.make_throwing_closure()
    return fn(nil)
end

-- 2) 调用前一个 .so 返回的闭包正常返回 nil 后，在本文件第 13 行抛错，位置必须指向本文件第 13 行。
function error_after_peer_closure_returns()
    local fn = ErrorLocation.make_returning_closure()
    local y = fn() + 1
    return y
end
