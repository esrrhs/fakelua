-- 文件级不允许 for-in 循环。
local t = {a = 1, b = 2}
for k, v in pairs(t) do
    local x = v
end
