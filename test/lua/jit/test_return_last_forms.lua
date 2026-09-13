-- return 处在块尾的各种合法形态：带结尾分号、do...end 里的 return、
-- 以及 return 之后紧跟块结束。
local function early(n)
    if n > 0 then
        return 1;
    end
    return 2
end

local function via_do_end()
    do
        return 3
    end
end

function test_return_last_forms()
    return early(1) + early(-1) + via_do_end()
end
