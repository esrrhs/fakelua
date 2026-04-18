-- ForLoop where the step is a parameter (T_DYNAMIC).
-- Even though begin (1) and end (9) are T_INT, the presence of a T_DYNAMIC
-- step causes all_int = false, so the loop variable is typed T_DYNAMIC and
-- the CVar for-loop path is taken.  The accumulator sum starts as T_INT and
-- degrades to T_DYNAMIC once it is assigned sum + i (T_DYNAMIC).
-- With step = 2:  i = 1, 3, 5, 7, 9  →  sum = 25.
function test(step)
    local sum = 0
    for i = 1, 9, step do
        sum = sum + i
    end
    return sum
end
