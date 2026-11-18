local function file_local_pair(value)
    return value, value + 1, value + 2
end

function test_loop_capture_contracts()
    local floating = {}
    for value = opaque_loop_value(0.5), opaque_loop_value(1.5), opaque_loop_value(0.5) do
        if value == 1.0 then continue end
        floating[#floating + 1] = function() return value end
    end
    assert(#floating == 2 and floating[1]() == 0.5 and floating[2]() == 1.5)
    local generic = {}
    for key, value in ipairs({10, 20, 30}) do
        if key == 2 then continue end
        generic[#generic + 1] = function() return key, value end
    end
    local a, b = generic[1]()
    local c, d = generic[2]()
    assert(a == 1 and b == 10 and c == 3 and d == 30)
    local first, second, third, missing = file_local_pair(10)
    local result = function() return first + second + third, missing end
    local total, absent = result()
    assert(total == 33 and absent == nil)
    return 1
end
