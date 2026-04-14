-- Insertion sort: sort array of 8 elements, return element at index k.
-- Exercises: for loop, while loop, table get/set, comparisons, local vars.
local function insertion_sort(t, n)
    for i = 2, n do
        local key = t[i]
        local j = i - 1
        while j >= 1 and t[j] > key do
            t[j + 1] = t[j]
            j = j - 1
        end
        t[j + 1] = key
    end
end

function test(k)
    local t = {42, 17, 56, 3, 99, 28, 11, 74}
    insertion_sort(t, 8)
    return t[k]
end

-- Return median of 5 elements (index 3 after sorting).
function test_median()
    local t = {9, 3, 7, 1, 5}
    insertion_sort(t, 5)
    return t[3]
end

-- Insertion sort on a float array; return sum as a float.
function test_float_sum()
    local t = {3.5, 1.2, 4.8, 2.1}
    insertion_sort(t, 4)
    local s = 0.0
    for i = 1, 4 do
        s = s + t[i]
    end
    return s
end
