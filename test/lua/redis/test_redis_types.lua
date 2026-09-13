package "RedisTest"

-- 测试 RESP 到 Lua 的基础类型映射：整数、数组、字符串；Redis 6+ 另测 nil 与 map
function on_connect(conn, err, success)
    conn.connected = (success == 1)
    conn.connect_err = err
end

function on_cmd(conn, err, result)
    conn.cmd_err = err
    conn.cmd_result = result
    conn.cmd_done = true
end

function test_types()
    local conn = redis.connect({
        host = "127.0.0.1",
        port = 6379,
        timeout_ms = 1000
    }, "on_connect")

    for i = 1, 1000 do
        runtime.tick()
        if conn.connected or conn.connect_err then break end
        os.sleep(1)
    end

    if not conn.connected then
        local err_str = conn.connect_err or ""
        print("failed to connect:", tostring(err_str))
        return 0
    end

    local prefix = "fl:redis:ty:" .. tostring(os.time())

    -- 1. INCR → 整数
    local ikey = prefix .. ":n"
    conn.cmd_done = false
    conn.cmd_err = nil
    conn.cmd_result = nil
    local incr = {}
    incr[1] = "INCR"
    incr[2] = ikey
    conn:command(incr, "on_cmd")
    for i = 1, 1000 do
        runtime.tick()
        if conn.cmd_done then break end
        os.sleep(1)
    end
    if not conn.cmd_done then
        print("INCR: callback never fired")
        conn:close()
        return 0
    end
    if conn.cmd_err ~= nil then
        if #conn.cmd_err > 0 then
            print("INCR failed:", tostring(conn.cmd_err))
            conn:close()
            return 0
        end
    end
    if conn.cmd_result ~= 1 then
        print("INCR result mismatch:", type(conn.cmd_result), tostring(conn.cmd_result))
        conn:close()
        return 0
    end

    -- 2. LPUSH + LRANGE → 数组
    local lkey = prefix .. ":list"
    conn.cmd_done = false
    conn.cmd_err = nil
    local l1 = {}
    l1[1] = "LPUSH"
    l1[2] = lkey
    l1[3] = "a"
    conn:command(l1, "on_cmd")
    for i = 1, 1000 do
        runtime.tick()
        if conn.cmd_done then break end
        os.sleep(1)
    end

    conn.cmd_done = false
    conn.cmd_err = nil
    local l2 = {}
    l2[1] = "LPUSH"
    l2[2] = lkey
    l2[3] = "b"
    conn:command(l2, "on_cmd")
    for i = 1, 1000 do
        runtime.tick()
        if conn.cmd_done then break end
        os.sleep(1)
    end

    conn.cmd_done = false
    conn.cmd_err = nil
    conn.cmd_result = nil
    local lr = {}
    lr[1] = "LRANGE"
    lr[2] = lkey
    lr[3] = "0"
    lr[4] = "-1"
    conn:command(lr, "on_cmd")
    for i = 1, 1000 do
        runtime.tick()
        if conn.cmd_done then break end
        os.sleep(1)
    end
    if not conn.cmd_done then
        print("LRANGE: callback never fired")
        conn:close()
        return 0
    end
    if conn.cmd_err ~= nil then
        if #conn.cmd_err > 0 then
            print("LRANGE failed:", tostring(conn.cmd_err))
            conn:close()
            return 0
        end
    end
    local arr = conn.cmd_result
    if type(arr) ~= "table" then
        print("LRANGE should be table, got:", type(arr))
        conn:close()
        return 0
    end
    if arr[1] ~= "b" or arr[2] ~= "a" then
        print("LRANGE values mismatch:", tostring(arr[1]), tostring(arr[2]))
        conn:close()
        return 0
    end

    -- 3. HSET + HGET → 字符串（RESP2 / RESP3 都适用）
    local hkey = prefix .. ":hash"
    conn.cmd_done = false
    conn.cmd_err = nil
    local hs = {}
    hs[1] = "HSET"
    hs[2] = hkey
    hs[3] = "name"
    hs[4] = "lua"
    conn:command(hs, "on_cmd")
    for i = 1, 1000 do
        runtime.tick()
        if conn.cmd_done then break end
        os.sleep(1)
    end

    conn.cmd_done = false
    conn.cmd_err = nil
    conn.cmd_result = nil
    local hg = {}
    hg[1] = "HGET"
    hg[2] = hkey
    hg[3] = "name"
    conn:command(hg, "on_cmd")
    for i = 1, 1000 do
        runtime.tick()
        if conn.cmd_done then break end
        os.sleep(1)
    end
    if not conn.cmd_done then
        print("HGET: callback never fired")
        conn:close()
        return 0
    end
    if conn.cmd_err ~= nil then
        if #conn.cmd_err > 0 then
            print("HGET failed:", tostring(conn.cmd_err))
            conn:close()
            return 0
        end
    end
    if conn.cmd_result ~= "lua" then
        print("HGET mismatch:", tostring(conn.cmd_result))
        conn:close()
        return 0
    end

    -- 4. Redis 6+：HELLO 3 后可测 nil 与 map；Redis 4/5 无 HELLO 则跳过
    conn.cmd_done = false
    conn.cmd_err = nil
    conn.cmd_result = nil
    local hello = {}
    hello[1] = "HELLO"
    hello[2] = "3"
    conn:command(hello, "on_cmd")
    for i = 1, 1000 do
        runtime.tick()
        if conn.cmd_done then break end
        os.sleep(1)
    end
    local can_map = false
    if conn.cmd_done then
        if conn.cmd_err == nil or #tostring(conn.cmd_err) == 0 then
            if type(conn.cmd_result) == "table" then
                can_map = true
            end
        end
    end
    if can_map then
        conn.cmd_done = false
        conn.cmd_err = nil
        conn.cmd_result = "sentinel"
        local missing = {}
        missing[1] = "GET"
        missing[2] = prefix .. ":missing"
        conn:command(missing, "on_cmd")
        for i = 1, 1000 do
            runtime.tick()
            if conn.cmd_done then break end
            os.sleep(1)
        end
        if not conn.cmd_done then
            print("GET missing: callback never fired")
            conn:close()
            return 0
        end
        if conn.cmd_err ~= nil then
            if #conn.cmd_err > 0 then
                print("GET missing failed:", tostring(conn.cmd_err))
                conn:close()
                return 0
            end
        end
        if conn.cmd_result ~= nil then
            print("GET missing should be nil, got:", type(conn.cmd_result), tostring(conn.cmd_result))
            conn:close()
            return 0
        end

        conn.cmd_done = false
        conn.cmd_err = nil
        conn.cmd_result = nil
        local ha = {}
        ha[1] = "HGETALL"
        ha[2] = hkey
        conn:command(ha, "on_cmd")
        for i = 1, 1000 do
            runtime.tick()
            if conn.cmd_done then break end
            os.sleep(1)
        end
        if not conn.cmd_done then
            print("HGETALL: callback never fired")
            conn:close()
            return 0
        end
        if conn.cmd_err ~= nil then
            if #conn.cmd_err > 0 then
                print("HGETALL failed:", tostring(conn.cmd_err))
                conn:close()
                return 0
            end
        end
        local mp = conn.cmd_result
        if type(mp) ~= "table" then
            print("HGETALL should be table, got:", type(mp))
            conn:close()
            return 0
        end
        if mp["name"] ~= "lua" then
            if mp[1] ~= "name" or mp[2] ~= "lua" then
                print("HGETALL name mismatch:", tostring(mp["name"]), tostring(mp[1]), tostring(mp[2]))
                conn:close()
                return 0
            end
        end
    end

    conn.cmd_done = false
    local delv = {}
    delv[1] = "DEL"
    delv[2] = ikey
    delv[3] = lkey
    delv[4] = hkey
    conn:command(delv, "on_cmd")
    for i = 1, 1000 do
        runtime.tick()
        if conn.cmd_done then break end
        os.sleep(1)
    end

    conn:close()
    return 1
end
