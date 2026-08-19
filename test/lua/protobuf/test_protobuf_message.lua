package "ProtobufTest"

-- 测试嵌套 message 往返
function test_message()
    local err = protobuf.load([[
        syntax = "proto3";
        message Address {
            string city = 1;
            int32 zip = 2;
        }
        message Person {
            string name = 1;
            int32 age = 2;
            Address addr = 3;
        }
    ]])
    if err ~= "ok" then return 0 end

    local msg = {
        name = "Alice",
        age = 30,
        addr = { city = "Beijing", zip = 100000 }
    }

    local bin = protobuf.encode("Person", msg)
    if not bin then return 0 end
    local d = protobuf.decode("Person", bin)

    if d.name ~= "Alice" then return 0 end
    if d.age ~= 30 then return 0 end
    if d.addr == nil then return 0 end
    if d.addr.city ~= "Beijing" then return 0 end
    if d.addr.zip ~= 100000 then return 0 end

    return 1
end
