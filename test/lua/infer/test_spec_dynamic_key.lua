-- 动态 key 访问（走 hash fallback）
function test_dynamic(k, v)
    local t = { x = 10 }
    t[k] = v
    return t.x
end
