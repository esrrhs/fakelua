-- String escape compile paths and extra string ops coverage.
function test_string_escapes()
    local s = "a\001b\n\tc\127d"
    if #s < 5 then return 1 end
    if string.byte(s, 2) ~= 1 then return 2 end
    return 0
end

function test_string_ops_extra()
    if string.lower("AbC") ~= "abc" then return 1 end
    if string.upper("AbC") ~= "ABC" then return 2 end
    if string.reverse("ab") ~= "ba" then return 3 end
    if string.rep("a", 3) ~= "aaa" then return 4 end
    if string.byte("ABC", 2) ~= 66 then return 5 end
    if string.char(65) ~= "A" then return 6 end
    if string.find("hello", "ll", 1, true) ~= 3 then return 7 end
    if tonumber("3.5") ~= 3.5 then return 8 end
    return 0
end
