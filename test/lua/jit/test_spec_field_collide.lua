function test_spec_field_collide()
    local t = {["a-b"] = 1, a_b = 2}
    if t["a-b"] ~= 1 then return 0 end
    if t.a_b ~= 2 then return 0 end
    return 1
end
