package "ProtobufTest"

-- 测试 .proto 解析：加载、错误处理、types/fields 查询
function test_load()
    -- 清空之前注册的 schema（保证测试独立）
    protobuf.load("")  -- 空文本不会注册任何内容

    -- 合法的 proto
    local err = protobuf.load([[
        syntax = "proto3";
        message TestMsg {
            int32 id = 1;
            string name = 2;
        }
    ]])
    if err ~= "ok" then return 0 end

    -- 验证 types() 返回了 TestMsg
    local types = protobuf.types()
    local found = false
    for _, t in ipairs(types) do
        if t == "TestMsg" then found = true end
    end
    if not found then return 0 end

    -- 验证 fields() 返回正确字段
    local fields = protobuf.fields("TestMsg")
    if #fields ~= 2 then return 0 end
    if fields[1].name ~= "id" then return 0 end
    if fields[1].number ~= 1 then return 0 end
    if fields[2].name ~= "name" then return 0 end
    if fields[2].number ~= 2 then return 0 end

    -- 语法错误应返回错误信息（非 "ok"）
    local bad = protobuf.load("this is not valid proto !!!")
    if bad == "ok" then return 0 end

    -- 嵌套 message
    local err2 = protobuf.load([[
        syntax = "proto3";
        message Outer {
            message Inner {
                int32 x = 1;
            }
            Inner inner = 1;
        }
    ]])
    if err2 ~= "ok" then return 0 end

    return 1
end
