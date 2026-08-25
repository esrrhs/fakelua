package "ProtobufTest"

-- 测试全部 18 种 scalar type 的往返
function test_scalar()
    local err = protobuf.load([[
        syntax = "proto3";
        message Scalar {
            double d = 1;
            float f = 2;
            int64 i64 = 3;
            uint64 u64 = 4;
            int32 i32 = 5;
            fixed64 fx64 = 6;
            fixed32 fx32 = 7;
            bool b = 8;
            string s = 9;
            bytes by = 10;
            uint32 u32 = 11;
            sfixed32 sfx32 = 12;
            sfixed64 sfx64 = 13;
            sint32 si32 = 14;
            sint64 si64 = 15;
        }
    ]])
    if err ~= "ok" then return 0 end

    local msg = {
        d = 3.14, f = 2.71, i64 = 9876543210, u64 = 1234567890,
        i32 = 2147483647, fx64 = 9999999999, fx32 = 4294967295,
        b = true, s = "hello", by = "binary" .. string.char(0) .. "data",
        u32 = 3000000000, sfx32 = -100, sfx64 = -200,
        si32 = -500, si64 = -1000
    }

    local bin = protobuf.encode("Scalar", msg)
    if not bin then return 0 end
    local d = protobuf.decode("Scalar", bin)

    if d.d < 3.13 or d.d > 3.15 then return 0 end
    if d.f < 2.70 or d.f > 2.72 then return 0 end
    if d.i64 ~= 9876543210 then return 0 end
    if d.u64 ~= 1234567890 then return 0 end
    if d.i32 ~= 2147483647 then return 0 end
    if d.fx64 ~= 9999999999 then return 0 end
    if d.fx32 ~= 4294967295 then return 0 end
    if d.b ~= true then return 0 end
    if d.s ~= "hello" then return 0 end
    if d.by ~= "binary" .. string.char(0) .. "data" then return 0 end
    if d.u32 ~= 3000000000 then return 0 end
    if d.sfx32 ~= -100 then return 0 end
    if d.sfx64 ~= -200 then return 0 end
    if d.si32 ~= -500 then return 0 end
    if d.si64 ~= -1000 then return 0 end

    return 1
end

function test_wrong_wire_type()
    local err = protobuf.load([[
        syntax = "proto3";
        message WireSkip {
            int32 x = 1;
        }
    ]])
    if err ~= "ok" then return 0 end
    -- field 1, wire type 1 (64-bit) + 8 bytes: skip, do not throw
    local bin = string.char(9) .. string.char(255, 255, 255, 255, 255, 255, 255, 255)
    local d = protobuf.decode("WireSkip", bin)
    if d.x ~= nil and d.x ~= 0 then return 0 end
    return 1
end
