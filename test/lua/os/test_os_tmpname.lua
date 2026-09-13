function test_os_tmpname()
    -- os.tmpname() returns a string
    local name = os.tmpname()
    if type(name) ~= "string" then return 0 end
    if #name == 0 then return 0 end

    -- two calls should return different names
    local name2 = os.tmpname()
    if name == name2 then return 0 end

    return 6000
end
