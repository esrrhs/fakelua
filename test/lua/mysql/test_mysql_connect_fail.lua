package "MysqlTest"

-- 尝试连接一个不存在的端口，验证 pcall 能捕获错误
function test_connect_fail()
    local ok, err = pcall(function()
        -- 端口 1 几乎必然无服务，连接应失败
        local conn = mysql.connect({
            host = "127.0.0.1",
            port = 1,
            user = "root",
            password = "irrelevant",
            db = "test"
        })
        -- 如果居然连上了，也手动断开
        if conn then conn:close() end
    end)

    -- 我们期望连接失败（ok == false），因为捕获到错误
    if ok then
        print("expected connect to fail, but it succeeded")
        return 0
    end

    -- err 应包含错误信息
    if type(err) ~= "string" or #err == 0 then
        print("expected error message, got:", type(err), tostring(err))
        return 0
    end

    return 1
end
