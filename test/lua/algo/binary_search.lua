-- Binary search in a fixed sorted table.
-- Returns the 1-based index of target, or 0 if not found.
-- Exercises: while loop, floor division, table access, comparisons.
function test(target)
    local t = {1, 3, 5, 7, 9, 11, 13, 15, 17, 19}
    local lo = 1
    local hi = 10
    while lo <= hi do
        local mid = (lo + hi) // 2
        if t[mid] == target then
            return mid
        elseif t[mid] < target then
            lo = mid + 1
        else
            hi = mid - 1
        end
    end
    return 0
end

-- Count comparisons made during binary search (instrumented version).
-- Exercises: local mutable variable incremented inside loop.
function test_steps(target)
    local t = {1, 3, 5, 7, 9, 11, 13, 15, 17, 19}
    local lo = 1
    local hi = 10
    local steps = 0
    while lo <= hi do
        local mid = (lo + hi) // 2
        steps = steps + 1
        if t[mid] == target then
            return steps
        elseif t[mid] < target then
            lo = mid + 1
        else
            hi = mid - 1
        end
    end
    return steps
end
