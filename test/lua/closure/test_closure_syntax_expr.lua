function make_adder(x)
    return function(y)
        return function(z)
            return x + y + z
        end
    end
end

function test()
    -- 1. 匿名函数表达式 (function() ... end)()
    local res1 = (function(a, b) return a * b end)(6, 7)

    -- 2. 链式调用 foo()()
    local res2 = make_adder(100)(20)(3)

    -- 3. 表字段与点号/方括号 Callee: tbl.add() 与 tbl["sub"]()
    local tbl = {}
    tbl.add = function(a, b) return a + b end
    tbl["sub"] = function(a, b) return a - b end

    local res3 = tbl.add(10, 5)
    local res4 = tbl["sub"](10, 5)

    return res1 + res2 + res3 + res4
end
