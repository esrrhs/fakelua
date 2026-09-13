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

function test_cycle_throw()
    local err = protobuf.load([[
        syntax = "proto3";
        message Node {
            int32 v = 1;
            Node child = 2;
        }
    ]])
    if err ~= "ok" then error("load failed") end
    local t = { v = 1 }
    t.child = t
    protobuf.encode("Node", t)
end

function test_decode_too_deep()
    local err = protobuf.load([[
        syntax = "proto3";
        message DeepNode {
            DeepNode child = 1;
        }
    ]])
    if err ~= "ok" then error("load failed") end
    local payload = ""
    local i = 1
    while i <= 70 do
        local n = #payload
        local lenbytes = ""
        while true do
            local b = n % 128
            n = math.floor(n / 128)
            if n == 0 then
                lenbytes = lenbytes .. string.char(b)
                break
            end
            lenbytes = lenbytes .. string.char(b + 128)
        end
        -- field 1, wire type 2 (length-delimited) → tag 10
        payload = string.char(10) .. lenbytes .. payload
        i = i + 1
    end
    protobuf.decode("DeepNode", payload)
end

function test_optional_nil()
    local err = protobuf.load([[
        syntax = "proto3";
        message M {
            optional int32 x = 1;
            int32 y = 2;
        }
    ]])
    if err ~= "ok" then return 0 end
    local bin = protobuf.encode("M", { y = 7 })
    if not bin then return 0 end
    local d = protobuf.decode("M", bin)
    if d.y ~= 7 then return 0 end
    return 1
end
