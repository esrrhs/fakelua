-- Test os.execute with empty string (checks shell availability)

function test_os_execute_empty()
    -- os.execute("") should return true (shell available)
    local r = os.execute("")
    if r ~= true then return 1 end

    return 5000
end
