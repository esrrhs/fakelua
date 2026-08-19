package "SerializeTest"

-- 跳过不支持的类型：表中的函数值应被跳过，其余字段正常往返
function skip_helper()
    -- 占位，用于构造一个函数值
end

function test_skip()
    local t = {
        num = 42,
        str = "keep",
        fn = skip_helper,   -- 函数，应被跳过
        ok = true
    }
    local bin = serialize.encode(t)
    if not bin then return 0 end
    local d = serialize.decode(bin)
    if d.num ~= 42 then return 0 end
    if d.str ~= "keep" then return 0 end
    if d.ok ~= true then return 0 end
    -- 函数被跳过，对应字段不存在
    if d.fn ~= nil then return 0 end
    return 1
end
