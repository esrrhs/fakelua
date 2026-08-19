package "ProtobufTest"

-- 测试 map<K,V> 往返
function test_map()
    local err = protobuf.load([[
        syntax = "proto3";
        message Bag {
            map<string, int32> items = 1;
            map<int32, string> id_to_name = 2;
        }
    ]])
    if err ~= "ok" then return 0 end

    local msg = {
        items = { sword = 1, shield = 2, potion = 5 },
        id_to_name = { [100] = "alice", [200] = "bob" }
    }

    local bin = protobuf.encode("Bag", msg)
    if not bin then return 0 end
    local d = protobuf.decode("Bag", bin)

    if d.items == nil then return 0 end
    if d.items.sword ~= 1 then return 0 end
    if d.items.shield ~= 2 then return 0 end
    if d.items.potion ~= 5 then return 0 end

    if d.id_to_name == nil then return 0 end
    if d.id_to_name[100] ~= "alice" then return 0 end
    if d.id_to_name[200] ~= "bob" then return 0 end

    return 1
end
