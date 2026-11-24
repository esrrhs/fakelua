package "HostErrorLocation"

function standard()
    local result = host_standard_error()
    return result
end

function unknown()
    local result = host_unknown_error()
    return result
end

function nested()
    local result = standard()
    return result
end

function protected_standard()
    local ok, message = pcall(standard)
    assert(not ok)
    return message
end

function compile_failure()
    host_compile_bad_source()
end

function success()
    return 42
end
