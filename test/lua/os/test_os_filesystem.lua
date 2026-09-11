function test_os_filesystem()
    local tmp = os.tmpname()
    if type(tmp) ~= "string" or #tmp == 0 then return 0 end
    if not os.exists(tmp) then return 0 end
    if not os.isfile(tmp) then return 0 end
    if os.isdir(tmp) then return 0 end
    local sz = os.filesize(tmp)
    if type(sz) ~= "number" or sz < 0 then return 0 end
    local mt = os.mtime(tmp)
    if type(mt) ~= "number" or mt <= 0 then return 0 end

    if os.basename(tmp) == "" then return 0 end
    if os.dirname(tmp) == nil then return 0 end
    if os.extension("foo.lua") ~= ".lua" then return 0 end
    local joined = os.join("a", "b", "c")
    if type(joined) ~= "string" or #joined == 0 then return 0 end
    local abs = os.absolute(tmp)
    if type(abs) ~= "string" or #abs == 0 then return 0 end
    local can = os.canonical(tmp)
    if type(can) ~= "string" or #can == 0 then return 0 end

    local dir = tmp .. "_dir"
    if os.mkdir(dir) ~= true then return 0 end
    if not os.isdir(dir) then return 0 end
    local nested = os.join(dir, "a", "b")
    if os.mkdir(nested) ~= true then return 0 end
    local names = os.listdir(dir)
    if type(names) ~= "table" then return 0 end
    if names[1] ~= "a" then return 0 end

    local dest = tmp .. ".copy"
    if os.copy(tmp, dest) ~= true then return 0 end
    if not os.exists(dest) then return 0 end

    local cwd = os.getcwd()
    if type(cwd) ~= "string" or #cwd == 0 then return 0 end
    if os.chdir(dir) ~= true then return 0 end
    local now = os.getcwd()
    os.chdir(cwd)
    if now == cwd then return 0 end

    os.remove(dest)
    os.remove(tmp)
    local nrm = os.remove_all(dir)
    if type(nrm) ~= "number" or nrm < 1 then return 0 end
    if os.exists(dir) then return 0 end
    if os.isfile("___no_such_file_xyz___") then return 0 end
    if os.filesize("___no_such_file_xyz___") ~= nil then return 0 end
    return 1
end
