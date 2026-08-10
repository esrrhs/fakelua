-- Test string.byte with range arguments (multi-value return)

function test_string_byte_multi()
    -- Single byte
    local b1 = string.byte("ABC", 1)
    if b1 ~= 65 then return 1 end  -- 'A' = 65

    -- Range: string.byte("ABC", 1, 3) returns 3 values
    local a, b, c = string.byte("ABC", 1, 3)
    if a ~= 65 then return 2 end  -- 'A'
    if b ~= 66 then return 3 end  -- 'B'
    if c ~= 67 then return 4 end  -- 'C'

    -- Out of bounds: start > end
    local r = string.byte("ABC", 3, 1)
    if r ~= nil then return 5 end

    -- Start beyond length returns nil
    local r2 = string.byte("ABC", 5, 6)
    if r2 ~= nil then return 6 end

    -- Negative indices
    local b2 = string.byte("ABC", -1)
    if b2 ~= 67 then return 7 end  -- last char 'C'

    return 5000
end
