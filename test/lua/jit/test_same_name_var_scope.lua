-- Test: same-name variables in different scopes should not interfere with dot access.
-- This specifically tests the case where a global variable and a local variable share
-- the same name, and a method call like x:method() must correctly identify whether
-- x is local or global based on the AST node pointer, not by name.

-- Global variable
x = {value = 10}

local function inner()
    -- Local variable with the same name as the global
    local x = {value = 20}
    return x.value
end

function test_same_name_var_scope()
    local a = inner()
    local b = x.value
    return a + b
end
