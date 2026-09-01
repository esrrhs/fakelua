package "CompressZstdCases"

-- 测试 zstd_decompress 空数据
function test_zstd_decompress_empty()
    local result = compress.zstd_decompress("")
    if result == nil then return 0 end
    return 1
end

-- 测试 zstd_decompress 无效数据（应报错）
function test_zstd_decompress_invalid()
    local ok, err = pcall(function()
        compress.zstd_decompress("invalid compressed data")
    end)
    if ok then return 0 end
    return 1
end

-- 测试 zstd_compress 越界 level
function test_zstd_compress_level_too_low()
    local data = "test data"
    local c = compress.zstd_compress(data, -1)
    if not c then return 0 end
    local d = compress.zstd_decompress(c)
    if d ~= data then return 0 end
    return 1
end

-- 测试 zstd_compress level 过高
function test_zstd_compress_level_too_high()
    local data = "test data"
    local c = compress.zstd_compress(data, 100)
    if not c then return 0 end
    local d = compress.zstd_decompress(c)
    if d ~= data then return 0 end
    return 1
end

-- 测试 zstd_compress 空数据
function test_zstd_compress_empty()
    local c = compress.zstd_compress("")
    if not c then return 0 end
    local d = compress.zstd_decompress(c)
    if d ~= "" then return 0 end
    return 1
end

-- 测试 zstd 大数据压缩解压
function test_zstd_large_data()
    local data = string.rep("abcdefghij", 100000)  -- 1MB
    local c = compress.zstd_compress(data)
    if not c then return 0 end
    local d = compress.zstd_decompress(c)
    if d ~= data then return 0 end
    return 1
end

-- 测试 zstd 各种 level
function test_zstd_various_levels()
    local data = string.rep("testdata", 1000)
    for level = 1, 22, 3 do
        local c = compress.zstd_compress(data, level)
        if not c then return 0 end
        local d = compress.zstd_decompress(c)
        if d ~= data then return 0 end
    end
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
