-- ForLoop with explicit integer step: exercises the
--   !for_loop->ExpStep() || for_loop->ExpStep()->EvalType() == T_INT
-- branch in TypeInferencer ForLoop handling.
-- All of begin (1), end (9), step (2) are T_INT, so the loop variable
-- is typed T_INT and the specialised int64_t for-loop path is taken.
-- i = 1, 3, 5, 7, 9  →  sum = 25.
function test()
    local sum = 0
    for i = 1, 9, 2 do
        sum = sum + i
    end
    return sum
end
