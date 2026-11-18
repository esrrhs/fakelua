local function rejects(label, fn, ...)
    assert(type(fn) == "function")
    local ok, err = pcall(fn, ...)
    assert(not ok and type(err) == "string", "expected rejection: " .. label)
end

function test_string_binary_roundtrips()
    for _, endian in ipairs({"<", ">", "="}) do
        for _, fmt in ipairs({"b", "B", "h", "H", "l", "L", "j", "J", "T", "i1", "i2", "i3", "i4", "i5", "i6", "i7", "i8", "I1", "I2", "I3", "I4", "I5", "I6", "I7", "I8"}) do
            local bytes = string.pack(endian .. fmt, 123)
            local value, nextpos = string.unpack(endian .. fmt, bytes)
            assert(value == 123 and nextpos == #bytes + 1)
            assert(#bytes == string.packsize(endian .. fmt))
            rejects("line 14", function(a1, a2) return string.unpack(a1, a2) end, endian .. fmt, "")
        end
        for _, fmt in ipairs({"f", "d"}) do
            local bytes = string.pack(endian .. fmt, 1.5)
            local value, nextpos = string.unpack(endian .. fmt, bytes)
            assert(value == 1.5 and nextpos == #bytes + 1)
            rejects("line 20", function(a1, a2) return string.unpack(a1, a2) end, endian .. fmt, "")
        end
        for _, fmt in ipairs({"z"}) do
            local bytes = string.pack(endian .. fmt, "payload")
            local value, nextpos = string.unpack(endian .. fmt, bytes)
            assert(value == "payload" and nextpos == #bytes + 1)
        end
    end
    for _, fmt in ipairs({"Xz", "Xc2"}) do
        assert(string.pack(fmt) == "")
        assert(string.packsize(fmt) == 0)
        assert(string.unpack(fmt, "") == 1)
    end
    local bytes = string.pack("!8 b Xd d c5", 7, 1.5, "ab")
    local a, b, c, nextpos = string.unpack("!8 b Xd d c5", bytes)
    assert(a == 7 and b == 1.5 and c == "ab\0\0\0" and nextpos == #bytes + 1)
    assert(string.unpack("<I2", "x\52\18", 2) == 4660)
    assert(string.unpack("<I2", "x\52\18", -2) == 4660)
    return 1
end

function test_string_binary_rejects_invalid_formats()
    for _, fmt in ipairs({"i0", "I17", "!3i4", "X", "X?"}) do
        rejects("line 38", function(a1, a2) return string.pack(a1, a2) end, fmt, 1)
        rejects("line 39", function(a1) return string.packsize(a1) end, fmt)
        rejects("line 40", function(a1, a2) return string.unpack(a1, a2) end, fmt, "1234567890123456")
    end
    rejects("line 42", function(a1, a2) return string.pack(a1, a2) end, "c2", "long")
    rejects("line 43", function(a1, a2) return string.pack(a1, a2) end, "f", true)
    rejects("line 44", function(a1, a2) return string.pack(a1, a2) end, "d", {})
    rejects("line 45", function(a1, a2) return string.pack(a1, a2) end, "z", true)
    rejects("line 46", function(a1, a2) return string.pack(a1, a2) end, "z", "a\0b")
    rejects("line 47", function(a1, a2) return string.unpack(a1, a2) end, "c8", "short")
    rejects("line 48", function(a1, a2) return string.unpack(a1, a2) end, "x", "")
    rejects("line 49", function(a1, a2) return string.unpack(a1, a2) end, "!8 b Xd d", "x")
    return 1
end


function test_additional_format_and_capture_contracts()
    assert(not pcall(function() return string.char("1.5") end))
    assert(not pcall(function() return string.char("9223372036854775808") end))
    -- X alignment and item size for l follow sizeof(long): 8 on LP64, 4 on Windows LLP64.
    local function size_after_byte(item)
        local align = item
        if align > 8 then
            align = 8
        end
        local pos = 1
        if pos % align ~= 0 then
            pos = pos + (align - pos % align)
        end
        return pos + item
    end
    assert(string.packsize("!8bXhh") == size_after_byte(string.packsize("h")))
    assert(string.packsize("!8bXll") == size_after_byte(string.packsize("l")))
    assert(string.packsize("!8bXjj") == size_after_byte(string.packsize("j")))
    assert(string.packsize("!8bXTT") == size_after_byte(string.packsize("T")))
    assert(string.packsize("!8bXff") == 8)
    assert(not pcall(function() return string.packsize("i0") end))
    assert(not pcall(function() return string.packsize("i17") end))
    assert(not pcall(function() return string.rep("x", 9223372036854775807) end))
    assert(string.format("%p", 0.0) == "0x0")
    assert(string.gsub("ab", "()", "[%1]") == "[1]a[2]b[3]")
    assert(string.gsub("ab", "()", {[1] = "x", [2] = "y", [3] = "z"}) == "xaybz")
    assert(string.gsub("a", "a", "%%") == "%")
    assert(not pcall(function() return string.gsub("a", ".", function() return {} end) end))
    local replacement = load("return 'R'")
    assert(string.gsub("abc", ".", replacement) == "RRR")
    return 1
end
