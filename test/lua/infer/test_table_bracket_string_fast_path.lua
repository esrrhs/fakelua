-- Table bracket access with string literal uses FlGetTableStrId fast path.
-- t["key"] emits FlGetTableStrId instead of the generic FlGetTable.
-- test() returns t["hello"] + t["world"] == 42 + 58 == 100.
function test()
    local t = {hello = 42, world = 58}
    return t["hello"] + t["world"]
end
