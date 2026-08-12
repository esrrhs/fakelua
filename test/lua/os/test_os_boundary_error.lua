function test_os_boundary_error()
    -- os.date: 非法参数（bool）报错
    os.date(true)
    return 5000
end
function test_os_boundary_error2()
    -- os.setlocale: 非法 category 报错
    os.setlocale("C", "bad")
    return 5000
end
function test_os_boundary_error3()
    -- os.time: 非法参数（string 而非 table）报错
    os.time("not-a-table")
    return 5000
end
