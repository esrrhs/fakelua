function test_os_setlocale()
    -- 空串是 setlocale(category, "")，按环境安装区域，不是查询
    local cur = os.setlocale("")
    if type(cur) ~= "string" then return 0 end

    -- set to C locale, returns previous
    local prev = os.setlocale("C")
    if type(prev) ~= "string" then return 0 end

    -- set back to C, should return "C"
    local cur2 = os.setlocale("C")
    if cur2 ~= "C" then return 0 end

    return 6000
end

function test_os_setlocale_nil_query()
    if os.setlocale("C", "all") == nil then return 1 end
    local q1 = os.setlocale(nil)
    if q1 ~= "C" then
        os.setlocale("C", "all")
        return 2
    end
    local q2 = os.setlocale()
    if q2 ~= "C" then
        os.setlocale("C", "all")
        return 3
    end
    local q3 = os.setlocale(nil, "all")
    if q3 ~= "C" then
        os.setlocale("C", "all")
        return 4
    end
    local q4 = os.setlocale(nil, "ctype")
    if q4 ~= "C" then
        os.setlocale("C", "all")
        return 5
    end
    local q5 = os.setlocale(nil, nil)
    os.setlocale("C", "all")
    if q5 ~= "C" then return 6 end
    return 5000
end

function test_os_setlocale_empty_env()
    -- 先记下环境区域，再强制成 C。空串必须装回环境，而不是把当前的 "C" 查询出来。
    local env = os.setlocale("", "all")
    if type(env) ~= "string" then
        os.setlocale("C", "all")
        return 1
    end
    if os.setlocale("C", "all") == nil then return 2 end
    if os.setlocale(nil, "all") ~= "C" then
        os.setlocale("C", "all")
        return 3
    end
    local applied = os.setlocale("", "all")
    local now = os.setlocale(nil, "all")
    os.setlocale("C", "all")
    if applied ~= env or now ~= env then return 4 end
    if env ~= "C" and applied == "C" then return 5 end
    return 5000
end
