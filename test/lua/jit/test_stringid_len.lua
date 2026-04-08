-- Test # operator on StringId type (Bug #10)
-- The # operator should work on all string types, including StringId

function test_len_short()
    local s = "hello world"
    return #s
end

function test_len_empty()
    local empty = ""
    return #empty
end

function test_len_long()
    local long = "abcdefghijklmnopqrstuvwxyz"
    return #long
end