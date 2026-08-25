package "CsvTest"

function test_decode_simple()
    local rows = csv.decode("a,b,c\n1,2,3")
    if type(rows) ~= "table" then return 0 end
    if #rows ~= 2 then return 0 end
    if type(rows[1]) ~= "table" then return 0 end
    if rows[1][1] ~= "a" or rows[1][2] ~= "b" or rows[1][3] ~= "c" then return 0 end
    if rows[2][1] ~= 1 or rows[2][2] ~= 2 or rows[2][3] ~= 3 then return 0 end
    return 1
end

function test_decode_single_row()
    local rows = csv.decode("hello,world")
    if type(rows) ~= "table" then return 0 end
    if #rows ~= 1 then return 0 end
    if rows[1][1] ~= "hello" or rows[1][2] ~= "world" then return 0 end
    return 1
end

function test_decode_single_column()
    local rows = csv.decode("a\nb\nc")
    if type(rows) ~= "table" then return 0 end
    if #rows ~= 3 then return 0 end
    if rows[1][1] ~= "a" or rows[2][1] ~= "b" or rows[3][1] ~= "c" then return 0 end
    return 1
end

function test_decode_quoted()
    local rows = csv.decode('"hello, world",foo')
    if type(rows) ~= "table" then return 0 end
    if #rows ~= 1 then return 0 end
    if rows[1][1] ~= "hello, world" then return 0 end
    if rows[1][2] ~= "foo" then return 0 end
    return 1
end

function test_decode_escaped_quotes()
    local rows = csv.decode('"a""b",c')
    if type(rows) ~= "table" then return 0 end
    if #rows ~= 1 then return 0 end
    if rows[1][1] ~= 'a"b' then return 0 end
    if rows[1][2] ~= "c" then return 0 end
    return 1
end

function test_decode_numbers()
    local rows = csv.decode("42,3.14,-100")
    if type(rows) ~= "table" then return 0 end
    if #rows ~= 1 then return 0 end
    if rows[1][1] ~= 42 then return 0 end
    if rows[1][2] < 3.13 or rows[1][2] > 3.15 then return 0 end
    if rows[1][3] ~= -100 then return 0 end
    return 1
end

function test_decode_empty_field()
    local rows = csv.decode("a,,c")
    if type(rows) ~= "table" then return 0 end
    if #rows ~= 1 then return 0 end
    if rows[1][1] ~= "a" then return 0 end
    if rows[1][2] ~= "" then return 0 end
    if rows[1][3] ~= "c" then return 0 end
    return 1
end

function test_decode_custom_sep()
    local rows = csv.decode("a;b;c\n1;2;3", ";")
    if type(rows) ~= "table" then return 0 end
    if #rows ~= 2 then return 0 end
    if rows[1][1] ~= "a" or rows[1][2] ~= "b" or rows[1][3] ~= "c" then return 0 end
    if rows[2][1] ~= 1 or rows[2][2] ~= 2 or rows[2][3] ~= 3 then return 0 end
    return 1
end

function test_encode_simple()
    local rows = {{"a", "b", "c"}, {"1", "2", "3"}}
    local s = csv.encode(rows)
    if s ~= "a,b,c\n1,2,3" then return 0 end
    return 1
end

function test_encode_quotes()
    local rows = {{'hello, world', "foo"}}
    local s = csv.encode(rows)
    if s ~= '"hello, world",foo' then return 0 end
    return 1
end

function test_encode_escaped_quotes()
    local rows = {{'a"b', "c"}}
    local s = csv.encode(rows)
    if s ~= '"a""b",c' then return 0 end
    return 1
end

function test_encode_numbers()
    local rows = {{42, 3.14}}
    local s = csv.encode(rows)
    if s ~= "42,3.14" then return 0 end
    return 1
end

function test_encode_custom_sep()
    local rows = {{"a", "b"}, {"c", "d"}}
    local s = csv.encode(rows, ";")
    if s ~= "a;b\nc;d" then return 0 end
    return 1
end

function test_roundtrip()
    local original = {{"name", "age", "city"}, {"Alice", "30", "Beijing"}, {"Bob", "25", "Shanghai"}}
    local encoded = csv.encode(original)
    local decoded = csv.decode(encoded)
    if #decoded ~= 3 then return 0 end
    if decoded[1][1] ~= "name" or decoded[1][2] ~= "age" or decoded[1][3] ~= "city" then return 0 end
    if decoded[2][1] ~= "Alice" or decoded[2][2] ~= 30 or decoded[2][3] ~= "Beijing" then return 0 end
    if decoded[3][1] ~= "Bob" or decoded[3][2] ~= 25 or decoded[3][3] ~= "Shanghai" then return 0 end
    return 1
end

function test_roundtrip_with_commas()
    local original = {{"hello, world", "foo"}, {"bar", "baz, qux"}}
    local encoded = csv.encode(original)
    local decoded = csv.decode(encoded)
    if #decoded ~= 2 then return 0 end
    if decoded[1][1] ~= "hello, world" then return 0 end
    if decoded[1][2] ~= "foo" then return 0 end
    if decoded[2][1] ~= "bar" then return 0 end
    if decoded[2][2] ~= "baz, qux" then return 0 end
    return 1
end

function test_unterminated_quote()
    csv.decode('"hello')
end
