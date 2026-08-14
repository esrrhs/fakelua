-- 文件级合法形态：local 定义、local function、function，以及无意义的空语句。
local base = 5;
local factor = {mul = 3}
local scaled = base * 2

local function helper(n)
    return n + base
end

function test_file_level_decls_only()
    return helper(scaled) + factor.mul
end
