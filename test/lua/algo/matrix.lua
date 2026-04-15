-- 3x3 matrix multiplication stored in flat tables (row-major, 1-indexed).
-- Returns trace (sum of diagonal elements) of the product C = A * B.
-- Exercises: nested for loops, table get/set with integer keys,
--            multiple local variables, arithmetic ops.
local function mat_mul(a, b, c)
    for i = 1, 3 do
        for j = 1, 3 do
            local s = 0
            for k = 1, 3 do
                s = s + a[(i - 1) * 3 + k] * b[(k - 1) * 3 + j]
            end
            c[(i - 1) * 3 + j] = s
        end
    end
end

-- A = [[1,2,3],[4,5,6],[7,8,9]], B = [[9,8,7],[6,5,4],[3,2,1]]
-- C = A*B, return trace(C) = C[1][1] + C[2][2] + C[3][3]
function test_trace()
    local a = {1, 2, 3, 4, 5, 6, 7, 8, 9}
    local b = {9, 8, 7, 6, 5, 4, 3, 2, 1}
    local c = {0, 0, 0, 0, 0, 0, 0, 0, 0}
    mat_mul(a, b, c)
    return c[1] + c[5] + c[9]
end

-- Return a specific cell of the product (1-indexed row r, col col_).
function test_cell(r, col_)
    local a = {1, 2, 3, 4, 5, 6, 7, 8, 9}
    local b = {9, 8, 7, 6, 5, 4, 3, 2, 1}
    local c = {0, 0, 0, 0, 0, 0, 0, 0, 0}
    mat_mul(a, b, c)
    return c[(r - 1) * 3 + col_]
end
