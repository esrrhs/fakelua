function test_snapshot_removal()
    local object = snapshot_object()
    local count = 0
    for key, value in pairs(object) do
        assert(value >= 1 and value <= 24)
        assert(key == "field_" .. tostring(value))
        count = count + 1
    end
    assert(count == 24)
    object.field_1 = nil
    object.field_12 = nil
    object.field_24 = nil
    assert(object.field_1 == nil and object.field_12 == nil and object.field_24 == nil)
    count = 0
    for key, value in pairs(object) do
        assert(value ~= 1 and value ~= 12 and value ~= 24)
        assert(key == "field_" .. tostring(value))
        count = count + 1
    end
    assert(count == 21)
    -- Grow an already materialized snapshot; existing keys/values must survive.
    for i = 25, 48 do object["field_" .. tostring(i)] = i end
    count = 0
    for key, value in pairs(object) do
        assert(key == "field_" .. tostring(value))
        count = count + 1
    end
    assert(count == 45)
    for i = 25, 48 do object["field_" .. tostring(i)] = nil end
    count = 0
    for key, value in pairs(object) do count = count + 1 end
    assert(count == 21)
    return 1
end
