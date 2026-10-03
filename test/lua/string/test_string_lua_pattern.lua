-- P1-5b 回归：string.find/match/gmatch/gsub 使用 Lua 5.4 模式（pattern），
-- 而不是 ECMAScript 正则。覆盖转义/字符类/集合/量词/锚点/捕获/%b/%f/gsub 替换。
-- 所有预期值以 PUC-Rio Lua 5.4/5.5 实跑为准。

local function check(cond, code)
    if not cond then return code end
end

function test_lua_pattern_escape()
    -- 标点转义：%. 匹配点号，%% 匹配百分号，%( %) %+ 等
    local a, b = string.find("../etc/passwd", "%.%.")
    if a ~= 1 or b ~= 2 then return 1 end
    if string.match("100%", "%%") ~= "%" then return 2 end
    if string.find("a+b", "%+") ~= 2 then return 3 end
    if string.match("x.y", "x%.y") ~= "x.y" then return 4 end
    -- 点号是任意单字节
    if string.match("a1b", "a.b") ~= "a1b" then return 5 end
    return 0
end

function test_lua_pattern_classes()
    if string.match("abc123", "%d+") ~= "123" then return 1 end
    if string.match("AbC", "%l+") ~= "b" then return 2 end
    if string.match("AbC", "%u+") ~= "A" then return 3 end
    if string.match("AbC", "%a+") ~= "AbC" then return 4 end
    -- %W 取反：非字母数字
    if string.match("ab1!cd", "%W") ~= "!" then return 5 end
    -- %x 十六进制
    if string.match("f0oBAR", "%x+") ~= "f0" then return 6 end
    if string.match("ZZZ9", "%X+") ~= "ZZZ" then return 7 end
    -- %s 空白 / %p 标点 / %c 控制符 / %g 可打印（不含空格）
    if string.match("a b", "%s") ~= " " then return 8 end
    if string.match("a,b", "%p") ~= "," then return 9 end
    if string.match("a\1b", "%c") ~= "\1" then return 10 end
    if string.match(" a", "%g") ~= "a" then return 11 end
    -- %z 零字节（Lua 5.1/5.5 保留类）
    if select(2, string.find("a\0b", "%z")) ~= 2 then return 12 end
    return 0
end

function test_lua_pattern_sets()
    if string.match("a-", "[-a]") ~= "a" then return 1 end
    if string.match("]x", "[]x]") ~= "]" then return 2 end
    if string.match("9a", "[%d_]") ~= "9" then return 3 end
    if string.match("a1_", "[%d_]+") ~= "1_" then return 4 end
    if string.match("9a", "[^%d]") ~= "a" then return 5 end
    if string.match("m", "[a-z]") ~= "m" then return 6 end
    if string.match("M", "[a-z]") ~= nil then return 7 end
    if string.match("7", "[0-9-]") ~= "7" then return 8 end
    -- 区间外的 '-' 在末尾是字面量
    if string.match("-", "[a-]") ~= "-" then return 9 end
    return 0
end

function test_lua_pattern_quantifiers()
    -- 惰性 '-'
    if string.match("a12b", "a.-b") ~= "a12b" then return 1 end
    if string.match("a12b34", "a(.-)b") ~= "12" then return 2 end
    -- '?' 可选
    if string.match("abc", "ab?c") ~= "abc" then return 3 end
    if string.match("ac", "ab?c") ~= "ac" then return 4 end
    -- 零宽重复的逐位行为（gsub/gmatch 不产生相邻重复空匹配）
    local r1, n1 = string.gsub("aaa", "b*", "x")
    if r1 ~= "xaxaxax" or n1 ~= 4 then return 5 end
    local r2, n2 = string.gsub("aaa", "a*", "x")
    if r2 ~= "x" or n2 ~= 1 then return 6 end
    local r3, n3 = string.gsub("abc", "a?", "x")
    if r3 ~= "xbxcx" or n3 ~= 3 then return 7 end
    local r4, n4 = string.gsub("", "", "x")
    if r4 ~= "x" or n4 ~= 1 then return 8 end
    -- 捕获组后面不含量词语义：(a)* 里的 '*' 是字面量
    if string.match("aaa", "(a)*") ~= nil then return 9 end
    return 0
end

function test_lua_pattern_anchors()
    if string.match("hello", "^h") ~= "h" then return 1 end
    if string.match("hello", "o$") ~= "o" then return 2 end
    if string.match("hello", "^o") ~= nil then return 3 end
    if string.match("hello", "h$") ~= nil then return 4 end
    -- 不在末尾的 '$' 是普通字符
    if string.match("a$b", "a$b") ~= "a$b" then return 5 end
    -- gsub 锚定只替换一次
    local r, n = string.gsub("aaa", "^a", "x")
    if r ~= "xaa" or n ~= 1 then return 6 end
    return 0
end

function test_lua_pattern_captures()
    -- 位置捕获 ()
    local a, b = string.match("hello", "()l()")
    if a ~= 3 or b ~= 4 then return 1 end
    -- 嵌套捕获
    local x, y, z = string.match("key=val", "((%w+)=(%w+))")
    if x ~= "key=val" or y ~= "key" or z ~= "val" then return 2 end
    -- 捕获反向引用 %1
    if string.match("abab", "(%w+)%1") ~= "ab" then return 3 end
    if string.match("abac", "(%w+)%1") ~= nil then return 4 end
    -- find 同时返回位置和捕获
    local s, e, cap = string.find("hello world", "(%w+)$")
    if s ~= 7 or e ~= 11 or cap ~= "world" then return 5 end
    -- match 无捕获返回整个匹配
    if string.match("2024-01-15", "%d+-%d+-%d+") ~= "2024-01-15" then return 6 end
    -- match 有多个捕获返回多值
    local cy, cm, cd = string.match("2024-01-15", "(%d+)-(%d+)-(%d+)")
    if cy ~= "2024" or cm ~= "01" or cd ~= "15" then return 7 end
    -- 空捕获串：组内可选项未消费时返回空串
    if string.match("b", "(a?)b") ~= "" then return 8 end
    return 0
end

function test_lua_pattern_frontier_balance()
    -- %f[set] 前沿模式（串首视作 '\0'，而 '\0' 属于 %S，所以首词前沿不触发）
    if string.match("  hello", "%f[%w]%w+") ~= "hello" then return 1 end
    local words = {}
    for w in string.gmatch("a b  c", "%f[%S]%S+") do words[#words + 1] = w end
    if #words ~= 2 or words[1] ~= "b" or words[2] ~= "c" then return 2 end
    -- %b() 平衡匹配
    if string.match("a(b(c)d)e", "%b()") ~= "(b(c)d)" then return 3 end
    local s, e = string.find("x(..(.).)y", "%b()")
    if s ~= 2 or e ~= 9 then return 4 end
    -- 自定义平衡字符
    if string.match("a[x[y]z]b", "%b[]") ~= "[x[y]z]" then return 5 end
    return 0
end

function test_lua_pattern_init_plain()
    -- init 起始位置（1-based，支持负数）
    local a = string.find("hello world", "o", 5)
    if a ~= 5 then return 1 end
    local b = string.find("hello world", "o", -5)
    if b ~= 8 then return 2 end
    -- match 的 init
    if string.match("hello world", "%w+", 7) ~= "world" then return 3 end
    -- plain=true 纯子串查找，元字符无特殊含义
    local s, e = string.find("a+b+c", "+", 1, true)
    if s ~= 2 or e ~= 2 then return 4 end
    return 0
end

function test_lua_pattern_gsub()
    -- 字符串替换：%0 整个匹配，%% 百分号，%1-%9 捕获
    local r1 = string.gsub("abc", "b", "[%0]")
    if r1 ~= "a[b]c" then return 1 end
    local r2 = string.gsub("a%b", "%%", "pct")
    if r2 ~= "apctb" then return 2 end
    local r3 = string.gsub("a1b2", "(%a)(%d)", "%2%1")
    if r3 ~= "1a2b" then return 3 end
    -- 函数替换：收捕获；nil/false 保留原文
    local r4 = string.gsub("hello 123", "%d+", function(d) return "[" .. d .. "]" end)
    if r4 ~= "hello [123]" then return 4 end
    local r5, c5 = string.gsub("aaa", "a", function() return nil end)
    if r5 ~= "aaa" or c5 ~= 3 then return 5 end
    local r6, c6 = string.gsub("aaa", "a", function() return false end)
    if r6 ~= "aaa" or c6 ~= 3 then return 6 end
    -- 表替换
    local r7 = string.gsub("a b c", "%a", { a = "X", c = "Z" })
    if r7 ~= "X b Z" then return 7 end
    local r8, c8 = string.gsub("k1 k9", "k(%d)", { ["1"] = "one", ["9"] = "nine" })
    if r8 ~= "one nine" or c8 ~= 2 then return 8 end
    -- 替换次数上限
    local r9, c9 = string.gsub("aaa aaa aaa", "aaa", "b", 2)
    if r9 ~= "b b aaa" or c9 ~= 2 then return 9 end
    return 0
end

function test_lua_pattern_gmatch()
    local out = {}
    for w in string.gmatch("a,b,,c", "[^,]*") do out[#out + 1] = w end
    if #out ~= 4 or out[1] ~= "a" or out[2] ~= "b" or out[3] ~= "" or out[4] ~= "c" then return 1 end
    local keys, vals = {}, {}
    for k, v in string.gmatch("a=1, b=2", "(%w)=(%d)") do
        keys[#keys + 1] = k
        vals[#vals + 1] = v
    end
    if #keys ~= 2 or keys[1] ~= "a" or vals[1] ~= "1" or keys[2] ~= "b" or vals[2] ~= "2" then return 2 end
    -- 空主体
    local n = 0
    for _ in string.gmatch("", "%w+") do n = n + 1 end
    if n ~= 0 then return 3 end
    return 0
end

function test_lua_pattern_charpattern()
    local pat = string.charpattern
    if type(pat) ~= "string" then return 1 end
    -- Lua 5.4 charpattern 匹配任意单字节，包括 '\0' 和 0xFF
    if string.match("hello", pat) ~= "h" then return 2 end
    if string.match("\0\255", pat) ~= "\0" then return 3 end
    local n = 0
    for _ in string.gmatch("12345", pat) do n = n + 1 end
    if n ~= 5 then return 4 end
    if string.match("", pat) ~= nil then return 5 end
    return 0
end

function test_lua_pattern_errors()
    -- 非法模式必须响亮报错（pcall 包一层 Lua 闭包）
    local cases = {
        function() return string.match("abc", "%") end,
        function() return string.find("abc", "[a") end,
        function() return string.gsub("abc", "%1", "x") end,
        function() return string.gsub("abc", ".", "%x") end,
        function() return string.gsub("abc", ".", "%") end,
        function() return string.match("abc", "%0") end
    }
    for i = 1, #cases do
        local ok_each = pcall(cases[i])
        if ok_each then return i end
    end
    -- 未闭合捕获在匹配成功提取时报 unfinished capture
    local ok_open = pcall(function() return string.match("ab", "(ab") end)
    if ok_open then return 6 end
    -- 合法的未匹配畸形片段不应报错（惰性解析，与 Lua 一致）
    local ok_lazy, ret = pcall(function() return string.match("zzz", "a[") end)
    if not ok_lazy or ret ~= nil then return 7 end
    return 0
end

function test()
    local rc
    rc = test_lua_pattern_escape()
    if rc ~= 0 then return rc + 100 end
    rc = test_lua_pattern_classes()
    if rc ~= 0 then return rc + 200 end
    rc = test_lua_pattern_sets()
    if rc ~= 0 then return rc + 300 end
    rc = test_lua_pattern_quantifiers()
    if rc ~= 0 then return rc + 400 end
    rc = test_lua_pattern_anchors()
    if rc ~= 0 then return rc + 500 end
    rc = test_lua_pattern_captures()
    if rc ~= 0 then return rc + 600 end
    rc = test_lua_pattern_frontier_balance()
    if rc ~= 0 then return rc + 700 end
    rc = test_lua_pattern_init_plain()
    if rc ~= 0 then return rc + 800 end
    rc = test_lua_pattern_gsub()
    if rc ~= 0 then return rc + 900 end
    rc = test_lua_pattern_gmatch()
    if rc ~= 0 then return rc + 1000 end
    rc = test_lua_pattern_charpattern()
    if rc ~= 0 then return rc + 1100 end
    rc = test_lua_pattern_errors()
    if rc ~= 0 then return rc + 1200 end
    return 0
end
