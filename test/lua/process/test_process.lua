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
