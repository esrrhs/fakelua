package "CompressErrorPaths"

-- 测试 zstd_decompress 空数据
function test_zstd_decompress_empty()
    local result = compress.zstd_decompress("")
    if result == nil then return 0 end
    return 1
end

-- 测试 zstd_decompress 无效帧
function test_zstd_decompress_invalid_frame()
    local ok, err = pcall(function()
        compress.zstd_decompress("invalid compressed data that is not a valid zstd frame")
    end)
    if ok then return 0 end
    return 1
end

-- 测试 zstd_compress level 边界
function test_zstd_compress_level_boundary()
    local data = "test data"
    -- level 过低
    local c1 = compress.zstd_compress(data, -100)
    if not c1 then return 0 end
    local d1 = compress.zstd_decompress(c1)
    if d1 ~= data then return 0 end
    -- level 过高
    local c2 = compress.zstd_compress(data, 100)
    if not c2 then return 0 end
    local d2 = compress.zstd_decompress(c2)
    if d2 ~= data then return 0 end
    return 1
end

-- 测试 zstd 大数据
function test_zstd_large_data()
    local data = string.rep("abcdefghij", 100000)  -- 1MB
    local c = compress.zstd_compress(data)
    if not c then return 0 end
    local d = compress.zstd_decompress(c)
    if d ~= data then return 0 end
    return 1
end

-- 测试 zstd 二进制数据含零
function test_zstd_binary_with_nulls()
    local data = string.char(0, 1, 0, 2, 0, 3, 0, 4)
    data = string.rep(data, 100)
    local c = compress.zstd_compress(data)
    if not c then return 0 end
    local d = compress.zstd_decompress(c)
    if d ~= data then return 0 end
    return 1
end

-- 测试 lz4_decompress 无效帧
function test_lz4_decompress_invalid_frame()
    local ok, err = pcall(function()
        compress.lz4_decompress("invalid lz4 frame data")
    end)
    if ok then return 0 end
    return 1
end

-- 测试 lz4_decompress 截断帧
function test_lz4_decompress_truncated()
    local data = string.rep("abcdefghij", 100)
    local compressed = compress.lz4_compress(data)
    local ok, err = pcall(function()
        compress.lz4_decompress(string.sub(compressed, 1, 16))
    end)
    if ok then return 0 end
    return 1
end

-- 测试 lz4_decompress 尾部垃圾数据
function test_lz4_decompress_trailing_garbage()
    local data = "hello"
    local compressed = compress.lz4_compress(data)
    local ok, err = pcall(function()
        compress.lz4_decompress(compressed .. "XXX")
    end)
    if ok then return 0 end
    return 1
end

-- 测试 lz4 大数据
function test_lz4_large_data()
    local data = string.rep("abcdefghij", 10000)
    local compressed = compress.lz4_compress(data)
    if not compressed then return 0 end
    local decompressed = compress.lz4_decompress(compressed)
    if decompressed ~= data then return 0 end
    return 1
end

-- 测试 lz4 二进制数据含零
function test_lz4_binary_with_nulls()
    local data = string.char(0, 1, 0, 2, 0, 3, 0, 4)
    data = string.rep(data, 100)
    local compressed = compress.lz4_compress(data)
    if not compressed then return 0 end
    local decompressed = compress.lz4_decompress(compressed)
    if decompressed ~= data then return 0 end
    return 1
end
