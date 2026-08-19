package "ProtobufTest"

-- 测试 repeated 字段（packed）往返
function test_repeated()
    local err = protobuf.load([[
        syntax = "proto3";
        message Numbers {
            repeated int32 vals = 1;
            repeated string names = 2;
        }
    ]])
    if err ~= "ok" then return 0 end

    local msg = {
        vals = {1, 2, 3, 100, -50, 9999},
        names = {"alice", "bob", "charlie"}
    }

    local bin = protobuf.encode("Numbers", msg)
    if not bin then return 0 end
    local d = protobuf.decode("Numbers", bin)

    if #d.vals ~= 6 then return 0 end
    if d.vals[1] ~= 1 then return 0 end
    if d.vals[4] ~= 100 then return 0 end
    if d.vals[5] ~= -50 then return 0 end
    if d.vals[6] ~= 9999 then return 0 end

    if #d.names ~= 3 then return 0 end
    if d.names[1] ~= "alice" then return 0 end
    if d.names[3] ~= "charlie" then return 0 end

    return 1
end
