-- Sibling blocks may reuse the same label name (Lua 5.4).
function test_goto_sibling_labels()
    local function go(x)
        if x then
            goto l
            ::l::
            return 1
        else
            goto l
            ::l::
            return 2
        end
    end
    if go(true) ~= 1 then return 0 end
    if go(false) ~= 2 then return 0 end
    return 1
end
