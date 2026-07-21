-- Test: same-name variables in different scopes with method-style call.
-- The bug was that var_to_def_map_ was scanned by name, so a local variable
-- with the same name as another variable in a different scope could cause
-- incorrect is_local detection during CompileFunctioncall.

local function outer()
    local t = {value = 10}
    return t.value
end

local function inner()
    -- Same name 't' but in a different scope — a different AST node
    local t = {value = 20}
    return t.value
end

function test_same_name_var_scope()
    return outer() + inner()
end
