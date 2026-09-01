package "LogAllLevels"

-- 测试 log.trace 带各种参数类型
function test_log_trace_various_types()
    log.trace("trace message")
    log.trace("value:", 42)
    log.trace("flag:", true)
    log.trace("nil:", nil)
    log.trace("float:", 3.14)
    return 1
end

-- 测试 log.debug 带各种参数类型
function test_log_debug_various_types()
    log.debug("debug message")
    log.debug("count:", 100, "name:", "test")
    log.debug("mixed:", 1, true, nil, 2.5)
    return 1
end

-- 测试 log.warn 带各种参数类型
function test_log_warn_various_types()
    log.warn("warning message")
    log.warn("memory low:", 1024, "MB")
    log.warn("status:", false, "code:", 500)
    return 1
end

-- 测试 log.error 带各种参数类型
function test_log_error_various_types()
    log.error("error message")
    log.error("code:", 404, "reason:", "not found")
    log.error("failed:", true, "retries:", 3)
    return 1
end

-- 测试 log.critical 带各种参数类型
function test_log_critical_various_types()
    log.critical("critical message")
    log.critical("system failure:", true, "level:", 5)
    return 1
end

-- 测试 log.info 多类型混合（覆盖 FormatArgs 分支）
function test_log_info_mixed_types()
    log.info("nil:", nil, "bool:", true, "int:", 42, "float:", 3.14, "str:", "hello")
    return 1
end

-- 测试 log.info 纯布尔值
function test_log_info_booleans()
    log.info("true:", true, "false:", false)
    return 1
end

-- 测试 log.info 纯 nil
function test_log_info_nil_only()
    log.info("value:", nil)
    return 1
end

-- 测试 log.info 纯浮点数
function test_log_info_float_only()
    log.info("pi:", 3.14159265358979323846)
    return 1
end

-- 测试 log.info 纯整数
function test_log_info_integer_only()
    log.info("answer:", 42)
    return 1
end

-- 测试所有日志级别切换
function test_log_all_level_transitions()
    log.set_level(0)  -- trace
    log.trace("trace after set_level(0)")
    log.set_level(1)  -- debug
    log.debug("debug after set_level(1)")
    log.set_level(2)  -- info
    log.info("info after set_level(2)")
    log.set_level(3)  -- warn
    log.warn("warn after set_level(3)")
    log.set_level(4)  -- error
    log.error("error after set_level(4)")
    log.set_level(5)  -- critical
    log.critical("critical after set_level(5)")
    log.set_level(2)  -- 恢复 info
    return 1
end

-- 测试 log.set_level 边界值
function test_log_set_level_boundary()
    log.set_level(0)
    log.set_level(5)
    log.set_level(100)  -- 高值应该被接受
    log.set_level(2)  -- 恢复
    return 1
end

-- 测试 log.set_level 错误参数
-- 测试 log.set_level 错误参数 (exception test, use GCC backend)
function test_log_set_level_invalid()
    log.set_level("invalid")
    return 0  -- should not reach here
end

-- 测试 log.set_level 无参数 (exception test, use GCC backend)
function test_log_set_level_no_arg()
    log.set_level()
    return 0  -- should not reach here
end

-- 测试 log.set_file 错误参数 (exception test, use GCC backend)
function test_log_set_file_invalid()
    log.set_file(123)
    return 0  -- should not reach here
end

-- 测试 log.set_file 无参数 (exception test, use GCC backend)
function test_log_set_file_no_arg()
    log.set_file()
    return 0  -- should not reach here
end

-- 测试 log.info 无参数 (exception test, use GCC backend)
function test_log_info_no_args()
    log.info()
    return 0  -- should not reach here
end
