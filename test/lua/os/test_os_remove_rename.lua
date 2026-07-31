function test_os_remove_rename()
    -- create a temp file path
    local tmp = os.tmpname()
    if type(tmp) ~= "string" then return 0 end

    -- write to it via execute (create the file)
    os.execute("touch " .. tmp)

    -- rename it
    local newname = tmp .. ".renamed"
    local r1 = os.rename(tmp, newname)
    if r1 ~= true then return 0 end

    -- remove the renamed file
    local r2 = os.remove(newname)
    if r2 ~= true then return 0 end

    -- remove non-existent file returns nil
    local r3 = os.remove("/tmp/fakelua_nonexistent_file_12345")
    if r3 ~= nil then return 0 end

    return 6000
end
