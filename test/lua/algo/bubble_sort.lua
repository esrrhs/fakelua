-- Bubble sort: sort a fixed 9-element array and return the element at index k.
-- Exercises: local functions, for loops, if/else, table get/set, multi-assign swap.
local function bubble_sort(t, n)
    for i = 1, n do
        for j = 1, n - i do
            if t[j] > t[j + 1] then
                local tmp = t[j]
                t[j] = t[j + 1]
                t[j + 1] = tmp
            end
        end
    end
end

function test(k)
    local t = {5, 3, 8, 1, 9, 2, 7, 4, 6}
    bubble_sort(t, 9)
    return t[k]
end

-- Return sum of all elements after sorting (sum is invariant, good sanity check).
function test_sum()
    local t = {5, 3, 8, 1, 9, 2, 7, 4, 6}
    bubble_sort(t, 9)
    local s = 0
    for i = 1, 9 do
        s = s + t[i]
    end
    return s
end
