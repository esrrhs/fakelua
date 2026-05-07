-- Specializable function with a for-in loop inside the body.
-- Exercises type_inferencer.cpp CollectReturnExps for ForIn (lines 868-869).
-- n is a math param (used in sum + n). The for-in loop iterates a table.
-- test(10) = 6 + 10 = 16.
function test(n)
    local sum = 0
    local t = {1, 2, 3}
    for k, v in pairs(t) do
        sum = sum + v
    end
    return sum + n
end
