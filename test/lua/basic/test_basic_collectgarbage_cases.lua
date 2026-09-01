package "BasicCollectgarbageCases"

-- 测试 collectgarbage count
function test_collectgarbage_count()
    local kb = collectgarbage("count")
    if type(kb) ~= "number" then return 0 end
    if kb <= 0 then return 0 end
    return 1
end

-- 测试 collectgarbage collect
function test_collectgarbage_collect()
    local r = collectgarbage("collect")
    return 1
end

-- 测试 collectgarbage stop
function test_collectgarbage_stop()
    local r = collectgarbage("stop")
    return 1
end

-- 测试 collectgarbage restart
function test_collectgarbage_restart()
    local r = collectgarbage("restart")
    return 1
end

-- 测试 collectgarbage step
function test_collectgarbage_step()
    local r = collectgarbage("step")
    return 1
end

-- 测试 collectgarbage setpause
function test_collectgarbage_setpause()
    local r = collectgarbage("setpause", 200)
    return 1
end

-- 测试 collectgarbage setstepmul
function test_collectgarbage_setstepmul()
    local r = collectgarbage("setstepmul", 200)
    return 1
end

-- 测试 collectgarbage 默认值
function test_collectgarbage_default()
    local kb = collectgarbage()
    if type(kb) ~= "number" then return 0 end
    return 1
end
