package "MysqlContractTest"

-- MySQL 回调契约测试（无需真实 MySQL 服务器：连到死端口验证回调行为）
-- 异步回调模型：函数名 + 绑定参数（纯数据），不支持内联闭包：
--   conn:query(sql, "on_result", arg1, arg2, ...)
-- 绑定参数在登记当下被序列化成独立字节串暂存，跨任意次顶层 Call 的 arena Reset
-- 都不丢；结果派发时在 tick 帧内反序列化，追加在固定参数 (conn, err, result) 之后。
-- 回调结束字节串即释放，不占用 const arena。
-- 跨帧可变状态挂在 conn 对象字段上（连接是 C++ 持有的 native 对象，Reset 不回收）。
-- 回归：
--   P0-1  非法回调（闭包/数字/空串）曾被静默转成空名、回调永不触发，现在必须响亮报错
--   P0-2  连接未就绪时 query 的回调曾被静默吞掉（Query 提前 return 不设 pending_result_）
--   P1-4  同连接飞行中再 query 曾直接报 "connection not ready"；现在排队，回调恰好一次

-- 死端口配置：连接必然失败，正好驱动错误终态路径
local dead_cfg = {
    host = "127.0.0.1",
    port = 1,
    user = "root",
    password = "x",
    db = "test",
    timeout_ms = 500
}

-- 观测状态全部挂在 conn 字段上；绑定参数用 tag 区分是哪一次登记。
function contract_on_connect(c, err, success, tag)
    if tag == "connect_tag" then
        c.cc = (c.cc or 0) + 1
    end
end

function contract_on_query(c, err, result, tag, n, ctx)
    if tag == "query_tag" then
        c.qc = (c.qc or 0) + 1
        c.qerr = err
        c.qtag = tag
        c.qn = n
        c.qctx = ctx
    end
end

-- 场景 1：连接未就绪时发起 query —— 连接失败后回调恰好收到一次错误；
-- 绑定的字符串/数字/表参数跨帧存活并按快照原样送达。
function test_query_closure_exactly_once()
    local conn = mysql.connect(dead_cfg, "contract_on_connect", "connect_tag")

    -- 连接尚未就绪：query 排队，连接进入错误终态后回调收到错误
    conn:query("SELECT 1", "contract_on_query", "query_tag", 7, { k = "v" })

    for i = 1, 2000 do
        runtime.tick()
        if conn.qc then break end
        os.sleep(1)
    end

    if conn.cc ~= 1 then
        print("connect callback count = ", conn.cc)
        return 0
    end
    if conn.qc ~= 1 then
        print("query callback count = ", conn.qc)
        return 0
    end
    if type(conn.qerr) ~= "string" or #conn.qerr == 0 then
        print("query callback missing error, got:", conn.qerr)
        return 0
    end
    if conn.qtag ~= "query_tag" then
        print("bound string lost, got:", conn.qtag)
        return 0
    end
    if conn.qn ~= 7 then
        print("bound number lost, got:", conn.qn)
        return 0
    end
    if type(conn.qctx) ~= "table" or conn.qctx.k ~= "v" then
        print("bound table lost, got:", type(conn.qctx))
        return 0
    end
    return 1
end

-- 场景 2：回调名传非字符串（含内联闭包）、绑定参数含闭包 —— 都必须响亮报错
function test_bad_callback_type()
    local conn = mysql.connect(dead_cfg, "contract_on_connect", "connect_tag")

    local ok1 = pcall(function() conn:query("SELECT 1", 123) end)
    if ok1 then return 0 end
    local ok2 = pcall(function() conn:query("SELECT 1", nil) end)
    if ok2 then return 0 end
    local ok3 = pcall(function() conn:query("SELECT 1", {}) end)
    if ok3 then return 0 end
    -- 内联闭包不再支持：异步回调无法安全持有临时 arena 上的闭包
    local ok4 = pcall(function() conn:query("SELECT 1", function() end) end)
    if ok4 then return 0 end
    -- 回调名合法，但绑定参数里夹带闭包：同样响亮报错，不能静默丢字段
    local ok5 = pcall(function() conn:query("SELECT 1", "contract_on_query", "x", function() end) end)
    if ok5 then return 0 end
    -- 连接回调同理
    local ok6 = pcall(function() mysql.connect(dead_cfg, function() end) end)
    if ok6 then return 0 end

    conn:close()
    return 1
end

-- 场景 3：pool:with —— 无健康连接时返回 nil（fn 不执行）；非函数参数报错。
-- pool:with 的函数是【同帧同步调用】、不跨 tick 暂存，因此仍支持内联闭包。
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

-- 给 C++ 单测用：返回一个会 error() 的闭包，用来验证 pool:with 抛错后仍归还连接。
function make_thrower()
    return function(c)
        error("intentional failure")
    end
end

-- 跨帧绑定参数（P0-1 的新形态）：绑定参数序列化后必须活过下一次顶层 Call 的
-- arena Reset。arm 在一次 Call 里登记回调后返回；宿主显式 Reset 临时 arena；
-- pump 是下一次 Call，再 tick 到回调。若字节串暂存失效，ctx 字段会坏掉或回调跑不起来。
-- 回调派回登记时的引擎：三引擎各自 arm/pump 闭环，文件级 local 各自独立也没问题。
-- cross_hits 只承载动态值（表字段 / nil），不会被推断成文件级数值常量。
local cross_hits = nil

function contract_on_cross(c, err, result, ctx)
    if ctx.tag == "ok" and ctx.n == 7 and type(err) == "string" then
        cross_hits = ctx.n
    else
        cross_hits = -1
    end
end

function arm_cross_frame()
    cross_hits = nil
    local conn = mysql.connect(dead_cfg, "contract_on_connect", "connect_tag")

    conn:query("SELECT 1", "contract_on_cross", { tag = "ok", n = 7 })
    return 1
end

function pump_cross_frame()
    for i = 1, 2000 do
        runtime.tick()
        if cross_hits ~= nil then break end
        os.sleep(1)
    end
    if cross_hits == nil then return 0 end
    return cross_hits
end
