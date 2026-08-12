-- Test basic error paths: wrong argument types must throw
-- Covers: error / assert / select / next / pairs / ipairs / collectgarbage / tonumber

function test_error_bad_arg()
    error(42)
end

function test_assert_bad_msg()
    assert(false, 42)
end

function test_select_bad_arg()
    select(true, 1, 2)
end

function test_next_bad_arg()
    next(42)
end

function test_pairs_bad_arg()
    pairs(42)
end

function test_ipairs_bad_arg()
    ipairs(42)
end

function test_collectgarbage_bad_arg()
    collectgarbage(42)
end

function test_tonumber_bad_base()
    tonumber("1", true)
end
