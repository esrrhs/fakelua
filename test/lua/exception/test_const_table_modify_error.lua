-- 修改 init 中创建的文件级表字段应抛出运行时异常（创建时 CONST_FLAG）。
local t = { a = 1, b = 2 }
local arr = { 30, 10, 20 }
local single_arr = { 42 }
local packed = table.pack(1, 2, 3)

local function make_num()
    return 1
end

local function make_tbl()
    return { val = 1 }
end

local const_num = make_num()
local const_func_tbl = make_tbl()

function test_modify_const()
    t.a = 100 -- 应该抛出 "attempt to modify a const table"
    return 0
end

function test_sort_const()
    table.sort(arr)
    return 0
end

function test_sort_single_const()
    table.sort(single_arr)
    return 0
end

function test_move_into_const()
    local src = { 99, 88 }
    table.move(src, 1, 2, 1, arr)
    return 0
end

function test_move_empty_into_const()
    local src = { 99, 88 }
    table.move(src, 2, 1, 1, arr)
    return 0
end

function test_insert_const()
    table.insert(arr, 99)
    return 0
end

function test_insert_float_pos_const()
    table.insert(arr, 1.0, 99)
    return 0
end

function test_remove_const()
    table.remove(arr)
    return 0
end

function test_remove_oob_valid_const()
    table.remove(arr, #arr + 1)
    return 0
end

function test_modify_packed_const()
    packed[1] = 999
    return 0
end

function test_copy_const_num_to_table()
    local m = {}
    m.x = const_num
    m.x = 2
    if const_num ~= 1 or m.x ~= 2 then
        return -1
    end
    return const_num
end

function test_copy_const_tbl_to_table_modify()
    local m = {}
    m.t = const_func_tbl
    m.t.val = 2
    return 0
end

