function test_table_boundary_error()
    -- table.concat: 表中含非 string/number 值（bool）报错
    table.concat({1, true, 3})
    return 5000
end
