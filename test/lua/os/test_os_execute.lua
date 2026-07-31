function test_os_execute()
    -- os.execute() with no args returns true (shell available)
    local r1 = os.execute()
    if r1 ~= true then return 0 end

    -- os.execute(nil) returns true
    local r2 = os.execute(nil)
    if r2 ~= true then return 0 end

    -- os.execute("exit 0") should succeed
    local r3 = os.execute("exit 0")
    if r3 ~= true then return 0 end

    return 6000
end
