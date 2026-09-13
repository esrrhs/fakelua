function test_string_rep_bad_sep()
    -- sep (arg #3) must be a string/number; Bool is invalid per standard Lua
    string.rep("a", 3, true)
    return 0
end
