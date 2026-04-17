function test_stringid_dynamic_get(k)
    local t = { abc = 11 }
    return t[k]
end

function test_stringid_dynamic_set(k, v)
    local t = { abc = 11 }
    t[k] = v
    return t.abc
end

function test_float_int_key_alias_set()
    local t = {}
    t[2.0] = 5
    return t[2]
end

function test_float_int_key_alias_get()
    local t = {}
    t[2] = 7
    return t[2.0]
end

function test_empty_table_get()
    local t = {}
    return t["x"]
end

function test_quick_delete_and_preserve()
    local t = { a = 1, b = 2, c = 3 }
    t["a"] = nil
    if t["a"] == nil then
        return t["c"]
    end
    return -1
end

function test_hash_delete_head_chain()
    local t = {
        [1] = 10,
        [17] = 170,
        [2] = 20,
        [3] = 30,
        [4] = 40,
        [5] = 50,
        [6] = 60,
        [7] = 70,
        [8] = 80,
        [9] = 90
    }
    t[1] = nil
    if t[1] == nil then
        return t[17]
    end
    return -1
end

function test_hash_delete_chain_node()
    local t = {
        [1] = 10,
        [17] = 170,
        [2] = 20,
        [3] = 30,
        [4] = 40,
        [5] = 50,
        [6] = 60,
        [7] = 70,
        [8] = 80,
        [9] = 90
    }
    t[17] = nil
    if t[17] == nil then
        return t[1]
    end
    return -1
end
