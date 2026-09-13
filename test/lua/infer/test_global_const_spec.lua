-- 文件级整数常量 OFFSET 与数学参数 n 共同参与算术。
-- n 因 n + OFFSET 具有算术改善而成为数学参数，
-- 生成 add_offset_0(int64_t n) 特化版本。
local OFFSET = 10

function add_offset(n)
    return n + OFFSET
end
