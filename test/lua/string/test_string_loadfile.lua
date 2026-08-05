function test_string_loadfile()
    -- 加载外部文件，编译后文件中定义的函数直接变为全局
    local ret = loadfile("./string/test_string_loadfile_helper.lua")
    if ret ~= nil then return 0 end

    -- 文件中定义的函数已注册为全局，可直接调用
    local sum = helper_add(10, 20)
    if sum ~= 30 then return 0 end

    local msg = helper_greet("world")
    if msg ~= "hello world" then return 0 end

    -- 加载不存在的文件应返回 nil
    local ret2 = loadfile("./string/nonexistent_file_xyz.lua")
    if ret2 ~= nil then return 0 end

    -- 验证 load/loadstring 支持数字源码 chunk (Lua 标准规范)
    local cl = load(100)
    if cl == nil then return 0 end

    return 7000
end
