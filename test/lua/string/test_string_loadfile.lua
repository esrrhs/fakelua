function test_string_loadfile()
    -- 加载外部文件，返回闭包
    local f = loadfile("./string/test_string_loadfile_helper.lua")
    if type(f) ~= "function" then return 0 end

    -- 执行加载的闭包，使文件中定义的函数变为全局
    f()

    -- 调用文件中定义的函数
    local sum = helper_add(10, 20)
    if sum ~= 30 then return 0 end

    local msg = helper_greet("world")
    if msg ~= "hello world" then return 0 end

    -- 加载不存在的文件应返回 nil
    local f2 = loadfile("./string/nonexistent_file_xyz.lua")
    if f2 ~= nil then return 0 end

    return 7000
end
