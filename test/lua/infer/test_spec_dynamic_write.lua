-- 动态写入 fallback（写 hash，读 spec + hash）
function test_dynamic_write()
    local t = { x = 1 }
    t["y"] = 2
    return t.x + t.y
end
