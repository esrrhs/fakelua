package "CompressTest"

function test_zstd_compress_decompress()
    local data = "Hello, World! This is a test of Zstd compression."
    local compressed = compress.zstd_compress(data)
    if not compressed or #compressed == 0 then
        return 0
    end
    local decompressed = compress.zstd_decompress(compressed)
    if decompressed ~= data then
        return 0
    end
    return 1
end

function test_zstd_level()
    local data = string.rep("testdata", 1000)
    -- Level 1 (fast)
    local c1 = compress.zstd_compress(data, 1)
    if not c1 then return 0 end
    -- Level 19 (best ratio)
    local c19 = compress.zstd_compress(data, 19)
    if not c19 then return 0 end
    -- Both should decompress to original
    if compress.zstd_decompress(c1) ~= data then return 0 end
    if compress.zstd_decompress(c19) ~= data then return 0 end
    return 1
end

function test_zstd_binary()
    local data = ""
    for i = 0, 255 do
        data = data .. string.char(i)
    end
    local compressed = compress.zstd_compress(data)
    if not compressed then return 0 end
    local decompressed = compress.zstd_decompress(compressed)
    if decompressed ~= data then return 0 end
    return 1
end

function test_zstd_large_data()
    local data = string.rep("abcdefghij", 10000)
    local compressed = compress.zstd_compress(data)
    if not compressed then return 0 end
    local decompressed = compress.zstd_decompress(compressed)
    if decompressed ~= data then return 0 end
    return 1
end
