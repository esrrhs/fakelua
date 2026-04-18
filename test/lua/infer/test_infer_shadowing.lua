-- Variable shadowing inside a do...end block.
-- The inner `local val` uses the `local` keyword and therefore creates a NEW
-- variable that only lives inside the do...end scope; it does NOT mutate the
-- outer val.  After the block the outer val (100) must still be accessible.
function test()
    local val = 100          -- outer val, T_INT
    do
        local val = "inner"  -- inner val shadows outer; different variable
        local temp = val     -- temp = inner val ("inner"), T_DYNAMIC
    end
    local res = val + 1      -- outer val is still 100 → res = 101
    return res
end
