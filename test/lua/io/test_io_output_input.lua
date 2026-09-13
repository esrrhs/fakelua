package "IoOutputInput"

-- 测试 io.output/io.input
function test_io_output_input()
    local f = io.open("test_io_tmp.txt", "w")
    if not f then return 0 end
    local out = io.output(f)
    if not out then return 0 end
    local inp = io.input(f)
    if not inp then return 0 end
    f:close()
    os.remove("test_io_tmp.txt")
    return 1
end
