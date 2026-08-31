package "LogTest"

-- 测试 log.info 基本输出
function test_log_info()
    log.info("hello from lua")
    return 1
end

-- 测试 log.debug 基本输出
function test_log_debug()
    log.debug("debug message")
    return 1
end

-- 测试 log.warn 基本输出
function test_log_warn()
    log.warn("warn message")
    return 1
end

-- 测试 log.error 基本输出
function test_log_error()
    log.error("error message")
    return 1
end

-- 测试多参数拼接
function test_log_multi_args()
    log.info("value:", 42, "name:", "alice")
    return 1
end

-- 测试数字格式化
function test_log_number()
    log.info("int:", 123, "float:", 3.14)
    return 1
end

-- 测试布尔和 nil
function test_log_bool_nil()
    log.info("bool:", true, "nil:", nil)
    return 1
end

-- 测试 set_level
function test_set_level()
    log.set_level(2)  -- Info
    return 1
end

-- 测试带 pcall 的日志（codegen 兼容性）
function test_log_pcall()
    local ok, err = pcall(function() log.info("inside pcall") end)
    if not ok then
        return 0
    end
    return 1
end

-- 测试 log.trace 基本输出
function test_log_trace()
    log.trace("trace message")
    return 1
end

-- 测试 log.critical 基本输出
function test_log_critical()
    log.critical("critical message")
    return 1
end

-- 测试 log.set_file
function test_log_set_file()
    log.set_file("test_fakelua_log_tmp.txt")
    log.info("write to file")
    return 1
end

-- 测试 log.set_level 错误参数（非整数）
function test_set_level_bad_arg()
    local ok, err = pcall(function() log.set_level("bad") end)
    if ok then
        return 0
    end
    return 1
end

-- 测试 log.info 无参数（应抛异常）
function test_log_info_no_args()
    local ok, err = pcall(function() log.info() end)
    if ok then
        return 0
    end
    return 1
end
