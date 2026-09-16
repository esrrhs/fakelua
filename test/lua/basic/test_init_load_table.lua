package "InitLoadTable"

-- Nested load() during __fakelua_init must not drop the const-alloc flag,
-- or the following file-level table would live on the temp arena and UAF after Call().
local _ = load("return 42")
local t = {10, 20, 30}

function test_init_load_table()
    if t[1] ~= 10 then return 0 end
    if t[2] ~= 20 then return 0 end
    if t[3] ~= 30 then return 0 end
    return 1
end
