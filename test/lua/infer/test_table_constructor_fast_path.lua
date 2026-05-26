-- Table constructor uses FlSetTableInt for sequential integer keys
-- and FlSetTableStrId for named fields.
-- test() returns t[2] + t.x == 20 + 100 == 120.
function test()
    local t = {10, 20, 30, x = 100}
    return t[2] + t.x
end
