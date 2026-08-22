package "CompressTest"

function test_lz4_compress_decompress()
    local data = "Hello, World! This is a test of LZ4 compression."
    local compressed = compress.lz4_compress(data)
    if not compressed or #compressed == 0 then
        return 0
    end
    local decompressed = compress.lz4_decompress(compressed)
    if decompressed ~= data then
        return 0
    end
    return 1
end

function test_lz4_empty()
    local data = ""
    local compressed = compress.lz4_compress(data)
    if not compressed then
        return 0
    end
    local decompressed = compress.lz4_decompress(compressed)
    if decompressed ~= data then
        return 0
    end
    return 1
end

function test_lz4_binary()
    local data = ""
    for i = 0, 255 do
        data = data .. string.char(i)
    end
    local compressed = compress.lz4_compress(data)
    if not compressed then
        return 0
    end
    local decompressed = compress.lz4_decompress(compressed)
    if decompressed ~= data then
        return 0
    end
    return 1
end

function test_lz4_large_data()
    local data = string.rep("abcdefghij", 10000)
    local compressed = compress.lz4_compress(data)
    if not compressed then
        return 0
    end
    local decompressed = compress.lz4_decompress(compressed)
    if decompressed ~= data then
        return 0
    end
    return 1
end
