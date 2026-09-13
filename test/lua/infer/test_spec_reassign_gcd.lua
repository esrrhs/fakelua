-- GCD using the iterative Euclidean algorithm.
-- Both parameters a and b are reassigned inside the while loop body,
-- exercising the "reassigned params are still specialized" feature.
function test(a, b)
    while b ~= 0 do
        local tmp = b
        b = a % b
        a = tmp
    end
    return a
end
