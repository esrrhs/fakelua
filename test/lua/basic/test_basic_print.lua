function test_basic_print()
    -- 测试 print 不崩溃
    print("test", 123, true, nil)
    print() -- 空打印
    print("single")
    return 5000
end
