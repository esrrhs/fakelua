package "MysqlContractTest"

-- MySQL 回调契约测试（无需真实 MySQL 服务器：连到死端口验证回调行为）
-- 回归：
--   P0-1  内联闭包回调曾被静默丢弃（CVarToString 转成空串，GetFunction("") 查不到）
--   P0-2  连接未就绪时 query 的回调曾被静默吞掉（Query 提前 return 不设 pending_result_）
--   P1-4  同连接飞行中再 query 曾直接报 "connection not ready"；现在排队，回调恰好一次

-- 场景 1：连接未就绪时用闭包发起 query —— 连接失败后回调恰好收到一次错误
function test_query_closure_exactly_once()
    local state = { connect_calls = 0, query_calls = 0, query_err = nil }

    local conn = mysql.connect({
        host = "127.0.0.1",
        port = 1,
        user = "root",
        password = "x",
        db = "test",
        timeout_ms = 500
    }, function(c, err, success)
        state.connect_calls = state.connect_calls + 1
    end)

    -- 连接尚未就绪：query 排队，连接进入错误终态后回调收到错误
    conn:query("SELECT 1", function(c, err, result)
        state.query_calls = state.query_calls + 1
        state.query_err = err
    end)

    for i = 1, 2000 do
        runtime.tick()
        if state.query_calls > 0 then break end
        os.sleep(1)
    end

    if state.connect_calls ~= 1 then
        print("connect callback count = ", state.connect_calls)
        return 0
    end
    if state.query_calls ~= 1 then
        print("query callback count = ", state.query_calls)
        return 0
    end
    if type(state.query_err) ~= "string" or #state.query_err == 0 then
        print("query callback missing error, got:", state.query_err)
        return 0
    end
    return 1
end

-- 场景 2：回调参数传非函数/非字符串名 —— 必须响亮报错（曾静默丢弃）
function test_bad_callback_type()
    local conn = mysql.connect({
        host = "127.0.0.1",
        port = 1,
        user = "root",
        password = "x",
        db = "test",
        timeout_ms = 500
    }, function(c, err, success) end)

    local ok1 = pcall(function() conn:query("SELECT 1", 123) end)
    if ok1 then return 0 end
    local ok2 = pcall(function() conn:query("SELECT 1", nil) end)
    if ok2 then return 0 end
    local ok3 = pcall(function() conn:query("SELECT 1", {}) end)
    if ok3 then return 0 end

    conn:close()
    return 1
end

-- 场景 3：pool:with —— 无健康连接时返回 nil（fn 不执行）；非函数参数报错
function test_pool_with()
    local pool = mysql_pool.create({
        host = "127.0.0.1",
        port = 1,
        user = "root",
        password = "x",
        db = "test",
        pool_size = 1,
        timeout_ms = 300
    })

    -- 没有可用连接：with 返回 nil
    local r = pool:with(function(c)
        return 42
    end)
    if r ~= nil then return 0 end

    -- 非函数参数必须报错
    local ok = pcall(function() pool:with(123) end)
    if ok then return 0 end

    pool:close()
    return 1
end
