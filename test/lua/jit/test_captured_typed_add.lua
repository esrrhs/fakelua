function test_captured_typed_add()
    local a = 10
    local function inner()
        local b = a + 1
        return b
    end
    return inner()
end

-- 内层改 upvalue 后，外层算术必须读 box 而不是过期的原生局部。
function test_captured_mutate()
    local a = 10
    local function inner()
        a = a + 1
    end
    inner()
    return a
end
