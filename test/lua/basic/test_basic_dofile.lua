function test_basic_dofile()
    -- dofile 加载辅助文件，其中定义的函数注册为全局后可直接调用
    dofile("./basic/test_basic_dofile_helper.lua")

    local sum = helper_add(10, 20)
    if sum ~= 30 then return 0 end

    local prod = helper_mul(6, 7)
    if prod ~= 42 then return 0 end

    -- 加载不存在的文件应静默返回 nil（不抛异常）
    local ret = dofile("./basic/nonexistent_file_xyz.lua")
    if ret ~= nil then return 0 end

    return 5000
end
