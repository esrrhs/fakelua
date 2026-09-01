package "CompressZstdEdge"

-- 测试 zstd 解压空输入 - 返回空结果 (normal test, use TCC backend)
function test_zstd_decompress_empty_input()
    local result = compress.zstd_decompress("")
    if result == nil then return 1 end
    if #result == 0 then return 1 end
    return 0
end

-- 测试 zstd 解压截断数据 (exception test, use GCC backend)
function test_zstd_decompress_truncated()
    local data = "Hello, World! This is test data for zstd."
    local compressed = compress.zstd_compress(data)
    -- 截断压缩数据
    local truncated = compressed:sub(1, #compressed / 2)
    compress.zstd_decompress(truncated)
    return 0  -- should not reach here
end

-- 测试 zstd 解压随机数据 (exception test, use GCC backend)
function test_zstd_decompress_random_data()
    local random_data = ""
    for i = 1, 100 do
        random_data = random_data .. string.char(math.random(0, 255))
    end
    compress.zstd_decompress(random_data)
    return 0  -- should not reach here
end

-- 测试 zstd 解压添加尾随垃圾数据 (exception test, use GCC backend)
function test_zstd_decompress_trailing_garbage()
    local data = "Hello, World!"
    local compressed = compress.zstd_compress(data)
    -- 添加尾随垃圾数据
    local with_garbage = compressed .. "GARBAGE_DATA"
    compress.zstd_decompress(with_garbage)
    return 0  -- should not reach here
end

-- 测试 zstd 压缩级别边界
function test_zstd_compress_level_min()
    local data = string.rep("test", 100)
    local c = compress.zstd_compress(data, 1)
    if not c then return 0 end
    if compress.zstd_decompress(c) ~= data then return 0 end
    return 1
end

-- 测试 zstd 压缩级别边界 - 最高级别
function test_zstd_compress_level_max()
    local data = string.rep("test", 100)
    local c = compress.zstd_compress(data, 22)  -- zstd 最大级别
    if not c then return 0 end
    if compress.zstd_decompress(c) ~= data then return 0 end
    return 1
end

-- 测试 zstd 压缩级别边界 - 超出范围
function test_zstd_compress_level_out_of_range()
    local data = "test"
    -- 级别 0 应该使用默认级别
    local c = compress.zstd_compress(data, 0)
    if not c then return 0 end
    if compress.zstd_decompress(c) ~= data then return 0 end
    return 1
end

-- 测试 zstd 大文件压缩
function test_zstd_large_file()
    local data = string.rep("abcdefghij", 100000)  -- 1MB
    local c = compress.zstd_compress(data)
    if not c then return 0 end
    if compress.zstd_decompress(c) ~= data then return 0 end
    return 1
end

-- 测试 zstd 二进制数据含空字节
function test_zstd_binary_with_nulls()
    local data = ""
    for i = 0, 255 do
        data = data .. string.char(i)
    end
    data = data .. "\0\0\0"  -- 额外的空字节
    local c = compress.zstd_compress(data)
    if not c then return 0 end
    if compress.zstd_decompress(c) ~= data then return 0 end
    return 1
end
