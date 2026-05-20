-- Math function with a for-in loop in the body.
-- Exercises CollectReturnExps lines 951-955: the ForIn statement is processed
-- by the switch in CollectReturnExps when BuildFunctionReturnCache is called
-- for a math function.
-- n is a math param because n * 2 shows arithmetic improvement.
-- test(10) == 20.
function test(n)
    for k, v in pairs({}) do
        -- empty for-in body, no return
    end
    return n * 2
end
