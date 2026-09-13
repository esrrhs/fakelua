function test()
    local obj = {}
    obj.val = 100
    obj.add = function(self, a, b)
        return self.val + a + b
    end

    -- 验证方法调用语法: obj:add(20, 30)
    local res1 = obj:add(20, 30)

    -- 链式创建对象的闭包方法
    local function make_obj(base)
        local o = { val = base }
        o.get_mul = function(self, factor)
            return self.val * factor
        end
        return o
    end

    local res2 = make_obj(10):get_mul(5)

    return res1 + res2
end
