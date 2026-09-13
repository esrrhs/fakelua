function test_os_getenv()
    -- PATH should exist on most systems
    local path = os.getenv("PATH")
    if type(path) ~= "string" then return 0 end
    if #path == 0 then return 0 end

    -- non-existent var returns nil
    local nope = os.getenv("FAKELUA_DOES_NOT_EXIST_12345")
    if nope ~= nil then return 0 end

    -- empty string var name returns nil
    local empty = os.getenv("")
    if empty ~= nil then return 0 end

    return 6000
end
