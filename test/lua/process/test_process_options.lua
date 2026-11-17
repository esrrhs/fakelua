function test_process_options()
    assert(not pcall(function() process.run(false) end))
    assert(not pcall(function() process.run({}) end))
    assert(not pcall(function() process.run({"env"}, false) end))
    assert(not pcall(function() process.run({"env"}, {cwd = false}) end))
    assert(not pcall(function() process.run({"env"}, {env = false}) end))
    assert(not pcall(function() process.run({"env"}, {env = {bad = {}}}) end))
    local previous = os.getenv("FAKELUA_MODULE_ENV_TEST")
    local path = os.getenv("PATH")
    assert(type(path) == "string")
    local output, errors, code = process.run({"env"}, {cwd = ".",
        env = {PATH = path, FAKELUA_MODULE_ENV_TEST = "module-env-marker"}})
    assert(code == 0 and errors == "")
    assert(string.find(output, "FAKELUA_MODULE_ENV_TEST=module-env-marker", 1, true))
    assert(os.getenv("FAKELUA_MODULE_ENV_TEST") == previous)
    return 1
end
