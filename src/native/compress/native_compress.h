#pragma once

#include "state/state.h"

namespace fakelua::compress {

// Register compression library:
//   compress.lz4_compress(data) / compress.lz4_decompress(data)
//   compress.zlib_compress(data, level?) / compress.zlib_decompress(data)
//   compress.gzip_compress(data, level?) / compress.gzip_decompress(data)
//   compress.zstd_compress(data, level?) / compress.zstd_decompress(data)
void RegisterCompressLibraryApi(State *s);

}  // namespace fakelua::compress
