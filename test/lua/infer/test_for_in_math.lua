-- Math function with a for-in loop in the body.
-- n is a math param because n * 2 shows arithmetic improvement.
-- The for-in statement is processed when building the function return cache.
-- test(10) == 20.
function test(n)
    for k, v in pairs({}) do
        -- empty for-in body, no return
    end
    return n * 2
end
