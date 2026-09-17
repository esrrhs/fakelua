-- if/else 两个 constructor 合并后 ["a-b"] 与 a_b 的 C 字段名都是 _s_a_b。
-- 必须 uniquify，否则 optional FL_SET_SPEC 会写到同一个 struct 字段。
function test_spec_field_merge_sanitize(c)
    local t
    if c then
        t = {["a-b"] = 1}
    else
        t = {a_b = 2}
    end
    local x = t["a-b"] or 0
    local y = t.a_b or 0
    return x + y
end
