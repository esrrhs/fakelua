利用lex和yacc创建自己的脚本语言，使得轻量级，易绑定

脚本特性：
@.代码风格类似lua
@.全部为函数
@.不支持复杂结构如vector，map等
@.后期加入类成员和类函数的语法
@.todo

示例：
-- func1 comment
func myfunc1(arg1, arg2)
	
	if arg1 > 10 then
		
		return arg2
		
	end
	
	return arg1
	
end

-- func2 comment
func myfunc2(arg1, arg2)
	
	while arg1 > 10 then
		
		arg1--;
		arg2++;
		
	end
	
	var arg3 = arg1 + arg2 * arg1
	
	return arg3
	
end


