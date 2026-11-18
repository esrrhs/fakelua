-- Host callbacks deliberately keep key types opaque to the compiler. This
-- exercises ordinary embedding calls, not a specialized constant-key fast path.
function test_dynamic_quick_keys()
    local keys = {"a", "b", "c", "d", "e", "f", "g", "h"}
    for removed = 1, 8 do
        local t = {}
        for i = 1, 8 do t[opaque_table_key(keys[i])] = i end
        for i = 1, 8 do
            assert(t[opaque_string_key(keys[i])] == i)
            t[opaque_string_key(keys[i])] = i + 10
            assert(t[opaque_table_key(keys[i])] == i + 10)
        end
        t[opaque_table_key(keys[removed])] = nil
        assert(t[opaque_string_key(keys[removed])] == nil)
        local count = 0
        for k, v in pairs(t) do
            assert(v >= 11 and v <= 18)
            count = count + 1
        end
        assert(count == 7)
        for i = 1, 8 do
            if i ~= removed then assert(t[opaque_string_key(keys[i])] == i + 10) end
        end
    end
    return 1
end

function test_dynamic_collision_chains()
    local t = {}
    -- These positive 64-bit integer keys share low^high hash bits, stay outside
    -- the array part, and force hash-node lookup/removal through a long chain.
    for i = 1, 40 do t[opaque_table_key(i * 4294967297)] = i end
    for i = 1, 40 do assert(t[opaque_table_key(i * 4294967297)] == i) end
    for _, i in ipairs({1, 20, 40}) do
        t[opaque_table_key(i * 4294967297)] = nil
        assert(t[opaque_table_key(i * 4294967297)] == nil)
    end
    t[opaque_table_key(99 * 4294967297)] = nil
    for i = 2, 39 do
        if i ~= 20 then
            assert(t[opaque_table_key(i * 4294967297)] == i)
            t[opaque_table_key(i * 4294967297)] = -i
            assert(t[opaque_table_key(i * 4294967297)] == -i)
        end
    end
    for i = 41, 80 do t[opaque_table_key(i * 4294967297)] = i end
    local count = 0
    for k, v in pairs(t) do count = count + 1 end
    assert(count == 77)
    return 1
end

function test_presized_move_and_mixed_sort()
    local source = {}
    for i = 1, 64 do source[i] = i end
    local destination = {}
    assert(table.move(source, 1, 64, 5, destination) == destination)
    for i = 1, 64 do assert(destination[i + 4] == i) end
    assert(destination[4] == nil and destination[69] == nil)
    local values = {1, 1.5, 0.25, 2}
    table.sort(values)
    assert(values[1] == 0.25 and values[2] == 1 and values[3] == 1.5 and values[4] == 2)
    assert(not pcall(function() return table.sort({{}, {}}) end))
    return 1
end


local function constructor_pair() return 11, 12 end

local function mixed_constructor(key)
    return {marker = 7, ["fixed"] = 8, [string.len("abcd")] = 9, [key] = 10, constructor_pair()}
end

local function write_unknown(table_value, key, value) table_value[key] = value end
local function write_float(table_value, index) table_value[index] = 1.25 end

function test_mixed_constructor_and_dynamic_store_contracts()
    local t = mixed_constructor(true)
    assert(t.marker == 7 and t.fixed == 8 and t[4] == 9 and t[true] == 10)
    assert(t[1] == 11 and t[2] == 12)
    assert(not pcall(function() write_unknown(false, "key", 1) end))
    assert(not pcall(function() write_unknown({}, nil, 1) end))
    local numbers = {}
    for i = 1, 8 do table.insert(numbers, i) end
    write_float(numbers, 2)
    assert(numbers[2] == 1.25 and #numbers == 8)
    local empty_key = {[""] = 17, other = 19}
    assert(empty_key[""] == 17 and empty_key.other == 19)
    return 1
end


function test_loaded_sort_comparator()
    local values = {4, 1, 3, 2}
    local compare = load("false")
    table.sort(values, compare)
    assert(#values == 4 and values[1] + values[2] + values[3] + values[4] == 10)
    table.sort(values)
    assert(values[1] == 1 and values[2] == 2 and values[3] == 3 and values[4] == 4)
    assert(not pcall(function() return table.unpack(false) end))
    return 1
end
