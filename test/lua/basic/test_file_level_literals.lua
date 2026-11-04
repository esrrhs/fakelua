-- File-level locals for LiteralFromExp float/unop paths in interp codegen.
-- These locals MUST stay in the same file as test_file_level_literals.
local cov_neg = -42
local cov_flt = 1.25
local cov_paren = (-3.5)

function test_file_level_literals()
    if cov_neg ~= -42 then return 1 end
    if cov_flt ~= 1.25 then return 2 end
    if cov_paren ~= -3.5 then return 3 end
    return 0
end

function test_bool_nil_locals()
    local a = nil
    local b = true
    local c = false
    if a ~= nil then return 1 end
    if b ~= true then return 2 end
    if c ~= false then return 3 end
    return 0
end

function test_empty_body()
    return 0
end
