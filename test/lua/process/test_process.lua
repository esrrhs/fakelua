package "ProcessTest"

function test_echo()
    local out, err, code = process.run({"echo", "hello fakelua"})
    if code ~= 0 then return 0 end
    if not string.contains(out, "hello fakelua") then return 0 end
    return 1
end

function test_stdin()
    local out, err, code = process.run({"cat"}, {stdin = "from stdin\n"})
    if code ~= 0 then return 0 end
    if not string.contains(out, "from stdin") then return 0 end
    return 1
end

function test_timeout()
    local out, err, code = process.run({"sleep", "5"}, {timeout_ms = 50})
    if code == 0 then return 0 end
    return 1
end

function test_missing()
    process.run({"/no/such/fakelua_process_bin_zzz"})
    return 0
end

function test_output_exact_8mb()
    local out, err, code = process.run({"head", "-c", "8388608", "/dev/zero"})
    if code ~= 0 then return 0 end
    if #out ~= 8388608 then return 0 end
    return 1
end

function test_output_over_8mb()
    process.run({"head", "-c", "8388609", "/dev/zero"})
    return 0
end
