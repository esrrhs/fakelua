function test_method_eval_order()
    local t = { f = function(self, x) return x end }
    local i = 0
    local function obj()
        i = i + 1
        return t
    end
    local function arg()
        return i
    end
    local r = obj():f(arg())
    if r ~= 1 then return 0 end
    return 1
end
