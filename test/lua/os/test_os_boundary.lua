function test_os_boundary()
    -- 1. os.getenv: 不存在的变量返回 nil
    if os.getenv("___no_such_var_xyz___") ~= nil then return 1 end

    -- 2. os.remove: 删除不存在的文件返回 nil
    local r = os.remove("___no_such_file_xyz___")
    if r ~= nil then return 2 end

    -- 3. os.rename: 重命名不存在的文件返回 nil
    local r2 = os.rename("___no_such_file_xyz___", "___also_no_such___")
    if r2 ~= nil then return 3 end

    return 5000
end
