-- Test NaN equality semantics (Bug #4)
-- NaN should never equal anything, including itself (IEEE 754 and Lua 5.4)

function test_nan_not_equal_self()
    local nan = 0.0 / 0.0
    return nan ~= nan
end

function test_nan_not_equal_one()
    local nan = 0.0 / 0.0
    return nan ~= 1.0
end

function test_nan_not_equal_zero()
    local nan = 0.0 / 0.0
    return nan ~= 0.0
end