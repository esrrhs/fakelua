package "ProtobufTest"

-- 测试 enum 往返
function test_enum()
    local err = protobuf.load([[
        syntax = "proto3";
        enum Color {
            RED = 0;
            GREEN = 1;
            BLUE = 2;
        }
        message Paint {
            Color color = 1;
            repeated Color palette = 2;
        }
    ]])
    if err ~= "ok" then return 0 end

    local msg = {
        color = 2,  -- BLUE
        palette = { 0, 1, 2 }  -- RED, GREEN, BLUE
    }

    local bin = protobuf.encode("Paint", msg)
    if not bin then return 0 end
    local d = protobuf.decode("Paint", bin)

    if d.color ~= 2 then return 0 end
    if #d.palette ~= 3 then return 0 end
    if d.palette[1] ~= 0 then return 0 end
    if d.palette[2] ~= 1 then return 0 end
    if d.palette[3] ~= 2 then return 0 end

    return 1
end
