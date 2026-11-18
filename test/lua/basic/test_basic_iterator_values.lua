-- Iterator triples must also work as values, not only in compiler-specialized for loops.
function test_iterator_values()
    local entries = { alpha = 11, beta = 22, [5] = 33, [true] = 44 }
    local iter, state, control = pairs(entries)
    assert(state == entries and control == nil)
    local seen = {}
    local count = 0
    while true do
        local key, value = iter(state, control)
        if key == nil then break end
        assert(seen[key] == nil)
        assert(value == entries[key])
        seen[key] = true
        control = key
        count = count + 1
    end
    assert(count == 4)
    assert(iter(state, control) == nil)

    local sequence = {10, 20, 30, nil, 50}
    local next_index, array, index = ipairs(sequence)
    assert(array == sequence and index == 0)
    local sum = 0
    for expected = 1, 3 do
        local key, value = next_index(array, index)
        assert(key == expected and value == expected * 10)
        index = key
        sum = sum + value
    end
    assert(sum == 60 and next_index(array, index) == nil)
    assert(next_index(array, index) == nil)
    local empty_pairs, empty_state, empty_control = pairs({})
    assert(empty_pairs(empty_state, empty_control) == nil)
    local empty_ipairs, empty_array, empty_index = ipairs({})
    assert(empty_ipairs(empty_array, empty_index) == nil)
    return 1
end
