package "CompressTest"

function test_zlib_compress_decompress()
    local data = "Hello, World! This is a test of zlib compression."
    local compressed = compress.zlib_compress(data)
    if not compressed or #compressed == 0 then
        return 0
    end
    local decompressed = compress.zlib_decompress(compressed)
    if decompressed ~= data then
        return 0
    end
    return 1
end

function test_zlib_level()
    local data = string.rep("testdata", 1000)
    -- Level 1 (fast)
    local c1 = compress.zlib_compress(data, 1)
    if not c1 then return 0 end
    -- Level 9 (best ratio)
    local c9 = compress.zlib_compress(data, 9)
    if not c9 then return 0 end
    -- Both should decompress to original
    if compress.zlib_decompress(c1) ~= data then return 0 end
    if compress.zlib_decompress(c9) ~= data then return 0 end
    -- NaN 等级不能 UB，应回落到默认等级
    local cnan = compress.zlib_compress(data, 0 / 0)
    if not cnan then return 0 end
    if compress.zlib_decompress(cnan) ~= data then return 0 end
    return 1
end

function test_zlib_binary()
    local data = ""
    for i = 0, 255 do
        data = data .. string.char(i)
    end
    local compressed = compress.zlib_compress(data)
    if not compressed then return 0 end
    local decompressed = compress.zlib_decompress(compressed)
    if decompressed ~= data then return 0 end
    return 1
end

function test_gzip_compress_decompress()
    local data = "Hello, World! This is a test of gzip compression."
    local compressed = compress.gzip_compress(data)
    if not compressed or #compressed == 0 then
        return 0
    end
    local decompressed = compress.gzip_decompress(compressed)
    if decompressed ~= data then
        return 0
    end
    return 1
end

function test_gzip_level()
    local data = string.rep("testdata", 1000)
    local c1 = compress.gzip_compress(data, 1)
    if not c1 then return 0 end
    local c9 = compress.gzip_compress(data, 9)
    if not c9 then return 0 end
    if compress.gzip_decompress(c1) ~= data then return 0 end
    if compress.gzip_decompress(c9) ~= data then return 0 end
    return 1
end

function test_gzip_concat_members()
    local a = compress.gzip_compress("AAA")
    local b = compress.gzip_compress("BBB")
    if compress.gzip_decompress(a .. b) ~= "AAABBB" then return 0 end
    return 1
end

function test_gzip_trailing_garbage()
    local c = compress.gzip_compress("hi")
    compress.gzip_decompress(c .. "XXX")
    return 0
end
