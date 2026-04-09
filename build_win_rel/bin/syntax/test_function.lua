function func_a()
end

function _G.a.b:func_b(a, b, c, ...)
    return a + b + c
end

local function func_c(...)
    local param_list = {...}
    for i = 1, #param_list do
        print(param_list[i])
    end
end
