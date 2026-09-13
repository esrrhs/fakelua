function test_os_setlocale()
    -- query current locale (empty string arg)
    local cur = os.setlocale("")
    if type(cur) ~= "string" then return 0 end

    -- set to C locale, returns previous
    local prev = os.setlocale("C")
    if type(prev) ~= "string" then return 0 end

    -- set back to C, should return "C"
    local cur2 = os.setlocale("C")
    if cur2 ~= "C" then return 0 end

    return 6000
end
