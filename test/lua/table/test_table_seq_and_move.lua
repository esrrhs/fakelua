-- Table sequence length, insert/remove, and move coverage.
function test_table_seq_and_move()
    local t = {10, 20, 30, 40, 50}
    if #t ~= 5 then return 1 end
    -- force hash path mixed with array
    t["k"] = "v"
    if t[1] ~= 10 or t.k ~= "v" then return 2 end
    -- table.move / insert / remove paths
    table.insert(t, 2, 15)
    if t[2] ~= 15 then return 3 end
    local r = table.remove(t, 2)
    if r ~= 15 then return 4 end
    local dst = {}
    table.move(t, 1, 3, 1, dst)
    if dst[1] ~= 10 or dst[3] ~= 30 then return 5 end
    -- sparse then fill for seq scan
    local s = {}
    s[1] = 1
    s[3] = 3
    s[2] = 2
    if #s < 2 then return 6 end
    return 0
end
