local function rejects(label, fn, ...)
    assert(type(fn) == "function")
    local ok, err = pcall(fn, ...)
    assert(not ok, "expected rejection: " .. label)
    assert(type(err) == "string")
    assert(#err > 0)
end

function test_string_missing_arguments()
    local unary = {function(a1) return string.len(a1) end, function(a1) return string.reverse(a1) end, function(a1) return string.lower(a1) end, function(a1) return string.upper(a1) end,
                   function(a1) return string.trim(a1) end, function(a1) return string.trim_left(a1) end, function(a1) return string.trim_right(a1) end,
                   function(a1) return string.byte(a1) end, function(a1) return string.format(a1) end, function(a1) return string.dump(a1) end, function(a1) return string.pack(a1) end,
                   function(a1) return string.packsize(a1) end, function(a1) return load(a1) end}
    assert(#unary == 13)
    for _, fn in ipairs(unary) do
        rejects("line 16", fn)
        rejects("line 17", fn, true)
    end
    local binary = {function(a1, a2) return string.sub(a1, a2) end, function(a1, a2) return string.rep(a1, a2) end, function(a1, a2) return string.split(a1, a2) end, function(a1, a2) return string.starts_with(a1, a2) end,
                    function(a1, a2) return string.ends_with(a1, a2) end, function(a1, a2) return string.contains(a1, a2) end, function(a1, a2) return string.iequals(a1, a2) end,
                    function(a1, a2) return string.icontains(a1, a2) end, function(a1, a2) return string.istarts_with(a1, a2) end, function(a1, a2) return string.iends_with(a1, a2) end,
                    function(a1, a2) return string.find(a1, a2) end, function(a1, a2) return string.match(a1, a2) end, function(a1, a2) return string.gmatch(a1, a2) end, function(a1, a2) return string.unpack(a1, a2) end}
    assert(#binary == 14)
    for _, fn in ipairs(binary) do
        rejects("line 25", fn)
        rejects("line 26", fn, "abc")
        rejects("line 27", fn, "abc", true)
    end
    rejects("line 29", function() return string.join() end)
    rejects("line 30", function(a1) return string.join(a1) end, {})
    rejects("line 31", function(a1, a2) return string.join(a1, a2) end, true, ",")
    rejects("line 32", function() return string.replace() end)
    rejects("line 33", function(a1) return string.replace(a1) end, "abc")
    rejects("line 34", function(a1, a2) return string.replace(a1, a2) end, "abc", "a")
    rejects("line 35", function() return string.gsub() end)
    rejects("line 36", function(a1) return string.gsub(a1) end, "abc")
    rejects("line 37", function(a1, a2) return string.gsub(a1, a2) end, "abc", "a")
    return 1
end

function test_string_integer_and_format_contracts()
    rejects("line 42", function(a1) return string.char(a1) end, -1)
    rejects("line 43", function(a1) return string.char(a1) end, 256)
    rejects("line 44", function(a1) return string.char(a1) end, 1.5)
    rejects("line 45", function(a1) return string.char(a1) end, true)
    rejects("line 46", function(a1, a2) return string.char(a1, a2) end, 65, 256)
    rejects("line 47", function(a1, a2) return string.char(a1, a2) end, 65, 1.5)
    rejects("line 48", function(a1, a2) return string.char(a1, a2) end, 65, true)
    rejects("line 49", function(a1, a2) return string.byte(a1, a2) end, "abc", 1.5)
    rejects("line 50", function(a1, a2, a3) return string.byte(a1, a2, a3) end, "abc", 1, false)
    rejects("line 51", function(a1) return string.format(a1) end, "%")
    rejects("line 52", function(a1, a2) return string.format(a1, a2) end, "%k", 1)
    rejects("line 53", function(a1, a2) return string.format(a1, a2) end, "%d", true)
    rejects("line 54", function(a1, a2) return string.format(a1, a2) end, "%f", {})
    rejects("line 55", function(a1, a2) return string.format(a1, a2) end, "%d", 1.5)
    rejects("line 56", function(a1, a2) return string.format(a1, a2) end, "%q", {})
    rejects("line 57", function(a1, a2) return string.format(a1, a2) end, "%q", function() end)
    assert(string.format("%s", true) == "true")
    rejects("line 59", function(a1, a2, a3) return string.gsub(a1, a2, a3) end, "abc", "a", "%")
    rejects("line 60", function(a1, a2, a3) return string.gsub(a1, a2, a3) end, "abc", "a", "%x")
    rejects("line 61", function(a1, a2, a3) return string.gsub(a1, a2, a3) end, "abc", "a", "%9")
    assert(string.byte(string.char(65.0)) == 65)
    assert(string.format("%q", true) == "true")
    assert(string.format("%q", nil) == "nil")
    assert(string.format("%d", "17") == "17")
    assert(string.format("%.1f", "1.5") == "1.5")
    assert(string.format("%q", 0 / 0) == "(0/0)")
    assert(string.format("%q", math.huge) == "1e9999")
    assert(string.format("%q", -math.huge) == "-1e9999")
    assert(string.format("%q", 1.5) == "0x1.8p+0")
    assert(string.format("%6.3s", "abcdef") == "   abc")
    return 1
end
