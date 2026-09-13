package "LogFormatTypes"

-- 测试 log.info 格式化 nil
function test_log_info_nil()
    log.info("value:", nil)
    return 1
end

-- 测试 log.info 格式化 boolean
function test_log_info_bool()
    log.info("flag:", true, "other:", false)
    return 1
end

-- 测试 log.info 格式化 integer
function test_log_info_integer()
    log.info("count:", 42)
    return 1
end

-- 测试 log.info 格式化 float
function test_log_info_float()
    log.info("pi:", 3.14159)
    return 1
end

-- 测试 log.info 格式化字符串
function test_log_info_string()
    log.info("name:", "alice")
    return 1
end

-- 测试 log.info 混合类型格式化
function test_log_info_mixed()
    log.info("id:", 100, "name:", "bob", "active:", true, "score:", 95.5)
    return 1
end

-- 测试 log.debug 格式化
function test_log_debug_format()
    log.debug("debug value:", 123)
    return 1
end

-- 测试 log.warn 格式化
function test_log_warn_format()
    log.warn("warning:", "low memory")
    return 1
end

-- 测试 log.error 格式化
function test_log_error_format()
    log.error("error code:", 500)
    return 1
end

-- 测试 log.trace 格式化
function test_log_trace_format()
    log.trace("trace info:", "details")
    return 1
end

-- 测试 log.critical 格式化
function test_log_critical_format()
    log.critical("critical:", "system failure")
    return 1
end

-- 测试 log.set_level 错误参数（非整数）
function test_log_set_level_bad_arg()
    local ok, err = pcall(function() log.set_level("invalid") end)
    if ok then return 0 end
    return 1
end

-- 测试 log.set_level 无参数
function test_log_set_level_no_arg()
    local ok, err = pcall(function() log.set_level() end)
    if ok then return 0 end
    return 1
end

-- 测试 log.set_file 无参数
function test_log_set_file_no_arg()
    local ok, err = pcall(function() log.set_file() end)
    if ok then return 0 end
    return 1
end

-- 测试 log.info 无参数
function test_log_info_no_args()
    local ok, err = pcall(function() log.info() end)
    if ok then return 0 end
    return 1
end
