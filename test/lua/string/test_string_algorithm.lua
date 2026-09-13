function test_string_algorithm()
    if string.trim("  hi  ") ~= "hi" then return 0 end
    if string.trim("\thi\n") ~= "hi" then return 0 end
    if string.trim("  ") ~= "" then return 0 end
    if string.trim("keep  inner") ~= "keep  inner" then return 0 end
    if string.trim(123) ~= "123" then return 0 end
    if string.trim_left("  hi  ") ~= "hi  " then return 0 end
    if string.trim_right("  hi  ") ~= "  hi" then return 0 end
    if string.trim_left("\thi\n") ~= "hi\n" then return 0 end
    if string.trim_right("\thi\n") ~= "\thi" then return 0 end
    if string.trim_left("hi") ~= "hi" then return 0 end
    if string.trim_right("hi") ~= "hi" then return 0 end
    if string.trim_left("  ") ~= "" then return 0 end
    if string.trim_right("  ") ~= "" then return 0 end

    local t = string.split("a,b,c", ",")
    if #t ~= 3 or t[1] ~= "a" or t[2] ~= "b" or t[3] ~= "c" then return 0 end
    local t2 = string.split("a::b::c", "::")
    if #t2 ~= 3 or t2[1] ~= "a" or t2[2] ~= "b" or t2[3] ~= "c" then return 0 end
    local t3 = string.split("a,,b", ",")
    if #t3 ~= 3 or t3[1] ~= "a" or t3[2] ~= "" or t3[3] ~= "b" then return 0 end
    local t4 = string.split("", ",")
    if #t4 ~= 1 or t4[1] ~= "" then return 0 end
    local t5 = string.split("only", ",")
    if #t5 ~= 1 or t5[1] ~= "only" then return 0 end
    if string.join(t, ",") ~= "a,b,c" then return 0 end
    if string.join(t2, "::") ~= "a::b::c" then return 0 end
    if string.join({"a", "", "b"}, ",") ~= "a,,b" then return 0 end
    if string.join({}, ",") ~= "" then return 0 end
    if string.join({"only"}, ",") ~= "only" then return 0 end
    if string.join({1, 2, 3}, "-") ~= "1-2-3" then return 0 end
    if string.join(string.split("x::y::z", "::"), "::") ~= "x::y::z" then return 0 end
    if string.join({"a", "b"}, "") ~= "ab" then return 0 end

    if not string.starts_with("hello", "he") then return 0 end
    if string.starts_with("hello", "lo") then return 0 end
    if not string.starts_with("hello", "") then return 0 end
    if not string.ends_with("hello", "lo") then return 0 end
    if string.ends_with("hello", "he") then return 0 end
    if not string.contains("hello", "ell") then return 0 end
    if string.contains("hello", "xyz") then return 0 end
    if not string.contains("hello", "") then return 0 end
    if not string.starts_with(12345, "12") then return 0 end
    if not string.ends_with(12345, "45") then return 0 end

    if string.replace("hello", "l", "x") ~= "hexxo" then return 0 end
    if string.replace("aaa", "a", "b") ~= "bbb" then return 0 end
    if string.replace("hello", "z", "x") ~= "hello" then return 0 end
    if string.replace("keep inner  spaces", "  ", " ") ~= "keep inner spaces" then return 0 end

    if not string.iequals("Hello", "hello") then return 0 end
    if string.iequals("Hello", "hallo") then return 0 end
    if not string.iequals("ABC", "abc") then return 0 end
    if not string.icontains("Hello", "ELL") then return 0 end
    if string.icontains("Hello", "xyz") then return 0 end
    if not string.icontains("Hello", "") then return 0 end
    if not string.istarts_with("Hello", "he") then return 0 end
    if string.istarts_with("Hello", "lo") then return 0 end
    if not string.iends_with("Hello", "LO") then return 0 end
    if string.iends_with("Hello", "HE") then return 0 end
    if not string.istarts_with("Hello", "") then return 0 end
    if not string.iends_with("Hello", "") then return 0 end
    return 1
end

function test_split_empty_sep()
    string.split("abc", "")
    return 0
end

function test_replace_empty_from()
    string.replace("abc", "", "x")
    return 0
end
