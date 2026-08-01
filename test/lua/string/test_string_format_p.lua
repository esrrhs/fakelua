function test_string_format_p()
    -- 测试 string.format("%p") 输出十六进制地址格式
    local ptr_str = string.format("%p", 0)
    if type(ptr_str) ~= "string" then return 0 end
    -- 应该包含 "0x" 前缀
    if not string.find(ptr_str, "0x") then return 0 end

    -- 测试非零值
    local ptr_str2 = string.format("%p", 255)
    if type(ptr_str2) ~= "string" then return 0 end
    if not string.find(ptr_str2, "0x") then return 0 end

    -- 测试与其他格式混合
    local mixed = string.format("ptr=%p, num=%d", 123, 42)
    if not string.find(mixed, "ptr=0x") then return 0 end
    if not string.find(mixed, "num=42") then return 0 end

    -- 测试 nil 值（应该输出某个指针地址）
    local nil_ptr = string.format("%p", nil)
    if type(nil_ptr) ~= "string" then return 0 end
    if #nil_ptr == 0 then return 0 end

    return 5000
end
