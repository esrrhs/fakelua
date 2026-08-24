# FakeLua Native Libraries

Detailed API reference for all built-in native libraries. Each module lives in its own subdirectory under `src/native/` with a `.h` and `.cpp` file pair.

> For a high-level overview, see the [main README](../README.md).

## Module List

| Module | Directory | Description |
|--------|-----------|-------------|
| basic | `basic/` | Global functions: `print`, `type`, `tostring`, `tonumber`, `select`, `error`, `assert`, `pcall`, `xpcall`, `next`, `pairs`, `ipairs`, `collectgarbage` |
| math | `math/` | Math functions: arithmetic, trigonometry, exponential/logarithm, random, constants |
| table | `table/` | Table operations: `insert`, `remove`, `concat`, `sort`, `pack`, `unpack`, `move`, `create` |
| string | `string/` | String operations: substring, case, pattern matching (ECMAScript regex), formatting, binary pack/unpack, serialization |
| os | `os/` | OS interface: time, date, environment, file operations, process execution |
| utf8 | `utf8/` | UTF-8 encoding/decoding: `char`, `codepoint`, `codes`, `len`, `offset` |
| io | `io/` | File I/O: open, close, read, write, seek, popen, standard streams |
| net | `net/` | TCP networking: server/client with framed protocols, custom parsers, async event dispatch |
| timer | `timer/` | Timers: one-shot, periodic heartbeat, driven by `tick()` |
| event | `event/` | Pub/sub event system: `on`, `once`, `off`, `emit`, `clear`, `clear_all` |
| compress | `compress/` | Compression: LZ4, zlib, gzip, Zstd |
| crypto | `crypto/` | Cryptography: MD5/SHA1/SHA256, hex/base64, AES/RC4/Blowfish/DES/3DES |
| csv | `csv/` | CSV decode/encode |
| json | `json/` | JSON encode/decode |
| mysql | `mysql/` | Async MySQL client: direct connect + connection pool |
| sqlite | `sqlite/` | SQLite3 wrapper: exec, prepared statements, synchronous |
| serialize | `serialize/` | Binary serialization with zigzag+varint encoding and string deduplication |
| protobuf | `protobuf/` | Runtime .proto parsing, standard protobuf3 wire encode/decode |
| object | `object/` | NativeObject Lua-side API: group management, object creation/lookup |

---

## Basic (Global Functions)

**File:** `basic/native_basic.h` · **Registration:** `RegisterBasicLibraryApi`

| Function | Args | Description |
|----------|------|-------------|
| `print(...)` | vararg | Print all arguments tab-separated to stdout with newline |
| `type(v)` | 1 | Return type name: `"nil"`, `"boolean"`, `"number"`, `"string"`, `"table"`, `"function"`, `"userdata"` |
| `tostring(v)` | 1 | Convert value to string |
| `tonumber(v [, base])` | 1-2 | Convert to number with optional base (2-36) |
| `select(n, ...)` | vararg | Select args from index `n`; `select("#", ...)` returns count |
| `error(msg [, level])` | 1-2 | Throw error with message and optional level |
| `assert(v, ...)` | vararg | Throw if `v` is falsy; otherwise return all args |
| `pcall(f, ...)` | vararg | Protected call; returns `true, result...` or `false, errmsg` |
| `xpcall(f, msgh, ...)` | vararg | Protected call with error handler |
| `next(t, ...)` | vararg | Next key-value pair in table traversal |
| `pairs(t)` | 1 | Generic traversal iterator |
| `ipairs(t)` | 1 | Integer index traversal iterator |
| `collectgarbage([opt])` | vararg | Only `"count"` returns memory KB; other options are no-ops |

---

## Math

**File:** `math/native_math.h` · **Registration:** `RegisterMathLibraryApi`

| Function | Args | Description |
|----------|------|-------------|
| `math.abs(x)` | 1 | Absolute value (handles INT64_MIN) |
| `math.floor(x)` | 1 | Floor |
| `math.ceil(x)` | 1 | Ceiling |
| `math.max(...)` | vararg | Maximum |
| `math.min(...)` | vararg | Minimum |
| `math.sqrt(x)` | 1 | Square root |
| `math.sin/cos/tan(x)` | 1 | Trigonometric functions |
| `math.asin/acos/atan(x)` | 1 | Inverse trigonometric |
| `math.atan2(y, x)` | 2 | Two-argument arctangent |
| `math.sinh/cosh/tanh(x)` | 1 | Hyperbolic functions |
| `math.exp(x)` | 1 | Exponential e^x |
| `math.log(x [, base])` | 1-2 | Logarithm with optional base |
| `math.log10(x)` | 1 | Base-10 logarithm |
| `math.pow(x, y)` | 2 | Power x^y |
| `math.fmod(x, y)` | 2 | Floating-point modulo |
| `math.ldexp(x, exp)` | 2 | x * 2^exp |
| `math.modf(x)` | 1 | Integer and fractional parts |
| `math.frexp(x)` | 1 | Mantissa and exponent |
| `math.deg(x)` | 1 | Radians to degrees |
| `math.rad(x)` | 1 | Degrees to radians |
| `math.copysign(x, y)` | 2 | Copy sign |
| `math.type(x)` | 1 | Returns `"integer"`, `"float"`, or nil |
| `math.tointeger(x)` | 1 | Convert to integer if lossless |
| `math.ult(x, y)` | 2 | Unsigned less-than comparison |
| `math.random(...)` | vararg | Random number: 0-arg [0,1), 1-arg [1,u], 2-arg [l,u] |
| `math.randomseed(...)` | vararg | Seed the RNG |

**Constants:** `math.pi`, `math.huge`, `math.maxinteger`, `math.mininteger`

---

## Table

**File:** `table/native_table.h` · **Registration:** `RegisterTableLibraryApi`

| Function | Args | Description |
|----------|------|-------------|
| `table.insert(t, [pos,] val)` | vararg | Insert value at position or end |
| `table.remove(t, [pos])` | vararg | Remove element at position or end, returns it |
| `table.concat(t, [sep, [i, [j]]])` | vararg | Concatenate elements with optional separator and range |
| `table.unpack(t, [i, [j]])` | vararg | Unpack elements in range |
| `table.pack(...)` | vararg | Pack arguments into table with field `n` |
| `table.move(t1, f, e, t, [t2])` | vararg | Move elements between tables |
| `table.sort(t, [comp])` | vararg | Sort in-place with optional comparator |
| `table.create(n, [val])` | vararg | Create table with pre-allocated size, optionally filled |

---

## String

**File:** `string/native_string.h` · **Registration:** `RegisterStringLibraryApi`

| Function | Args | Description |
|----------|------|-------------|
| `string.len(s)` | 1 | Byte length |
| `string.sub(s, i, [j])` | 2-3 | Substring with 1-based indices, supports negative |
| `string.rep(s, n, [sep])` | 2-3 | Repeat string n times with optional separator |
| `string.reverse(s)` | 1 | Reverse string |
| `string.lower(s)` | 1 | ASCII lowercase |
| `string.upper(s)` | 1 | ASCII uppercase |
| `string.byte(s, [i, [j]])` | vararg | Byte values in range |
| `string.char(...)` | vararg | Characters from code points 0-255 |
| `string.format(fmt, ...)` | vararg | Formatted output (supports `%s %d %i %u %x %X %o %f %e %E %g %G %c %q %p`) |
| `string.find(s, pattern, [init, [plain]])` | vararg | Regex or plain substring search; returns positions + captures |
| `string.match(s, pattern, [init])` | vararg | Regex match; returns captures or full match |
| `string.gmatch(s, pattern)` | 2 | Iterator for regex matches |
| `string.gsub(s, pattern, repl, [n])` | vararg | Regex substitution; supports string/function/table replacements |
| `string.dump(f, [strip])` | vararg | Serialize closure to binary string |
| `load(source, ...)` | vararg | Compile Lua source string into closure |
| `loadstring(s, ...)` | vararg | Alias for `load` |
| `loadfile(file, ...)` | vararg | Load and compile Lua file |
| `string.pack(fmt, ...)` | vararg | Binary pack (Lua 5.3 format) |
| `string.packsize(fmt)` | 1 | Compute packed size for format |
| `string.unpack(fmt, s, [pos])` | vararg | Binary unpack (Lua 5.3 format) |

> ⚠️ `string.find`/`match`/`gmatch`/`gsub` use **ECMAScript regex** (`std::regex::ECMAScript`), not Lua patterns. See [Regex Matching](../README.md#regex-matching-uses-ecmascript-syntax-not-lua-patterns) in the main README.

---

## OS

**File:** `os/native_os.h` · **Registration:** `RegisterOsLibraryApi`

| Function | Args | Description |
|----------|------|-------------|
| `os.clock()` | 0 | CPU time in seconds |
| `os.date([fmt, [time]])` | vararg | Formatted date/time string; `"*t"` returns table `{year, month, day, hour, min, sec, wday, yday, isdst}` |
| `os.difftime(t2, t1)` | 2 | Difference between two timestamps |
| `os.execute([cmd])` | vararg | Execute shell command; returns `(status_bool_or_nil, "exit"|"signal"|"error", code)` triple |
| `os.exit([code, [close]])` | vararg | Terminate process |
| `os.getenv(name)` | 1 | Get environment variable |
| `os.remove(filename)` | 1 | Delete file |
| `os.rename(old, new)` | 2 | Rename file |
| `os.setlocale(locale, [cat])` | vararg | Set/query locale |
| `os.time([table])` | vararg | Current time or timestamp from table |
| `os.tmpname()` | 0 | Generate a safe temporary file name |

---

## UTF-8

**File:** `utf8/native_utf8.h` · **Registration:** `RegisterUtf8LibraryApi`

| Function | Args | Description |
|----------|------|-------------|
| `utf8.char(...)` | vararg | Code points to UTF-8 string |
| `utf8.codepoint(s, [i, [j]])` | vararg | Code points in range |
| `utf8.codes(s)` | 1 | Simplified iterator support |
| `utf8.len(s, [i, [j]])` | vararg | Character count; returns nil + pos on invalid byte |
| `utf8.offset(s, n, [i])` | vararg | Byte position of n-th character |

**Constant:** `utf8.charpattern`

---

## IO

**File:** `io/native_io.h` · **Registration:** `RegisterIoLibraryApi`

| Function | Args | Description |
|----------|------|-------------|
| `io.open(filename, [mode])` | vararg | Open file; returns file object or nil, err, errno |
| `io.close([file])` | vararg | Close file (default: flushes stdout) |
| `io.read(...)` | vararg | Read from stdin with format(s) |
| `io.write(...)` | vararg | Write to stdout |
| `io.flush()` | 0 | Flush stdout |
| `io.type(v)` | 1 | Returns `"file"`, `"closed file"`, or nil |
| `io.tmpfile()` | 0 | Create temp file |
| `io.popen(cmd, [mode])` | vararg | Open process pipe |
| `io.input([file])` | vararg | Set/get default input file |
| `io.output([file])` | vararg | Set/get default output file |
| `io.lines([file, ...])` | vararg | Open file and return line iterator |
| `io.stdin/stdout/stderr` | 0 | Standard stream file objects |

**File object methods** (type `iofile`):

| Method | Description |
|--------|-------------|
| `file:read([format...])` | Read with format(s) |
| `file:write(...)` | Write values, returns self |
| `file:flush()` | Flush buffer |
| `file:close()` | Close file |
| `file:seek([whence, [offset]])` | Seek; returns position |
| `file:setvbuf(mode, [size])` | Set buffering (`"no"`, `"full"`, `"line"`) |
| `file:lines()` | Line iterator closure |

---

## Net

**File:** `net/native_net.h` · **Registration:** `RegisterNetLibraryApi`

**Config table fields:** `ip`, `port`, `maxconn`, `backlog`, `nonblocking`, `nodelay`, `keepalive`, `framer`, `parser`, `fixed_len`

**Framer protocols:**

| Protocol | Description |
|----------|-------------|
| `header4` / `header4_be` | 4-byte big-endian length header (default) |
| `header4_le` | 4-byte little-endian length header |
| `header2` / `header2_be` | 2-byte big-endian length header |
| `header2_le` | 2-byte little-endian length header |
| `line` | Newline delimited, auto-stripped |
| `fixed` | Fixed-length (requires `fixed_len = N`) |
| `raw` | Raw passthrough |

**Custom parser:** `parser = "Package.func"` (Lua) or `custom_parser_fn`/`custom_encoder_fn` (C++ `NetConfig`)

| Function/Method | Description |
|----------|-------------|
| `net.server(config)` | Create TCP server |
| `net.client(config)` | Create TCP client |
| `obj:dispatch(func_name)` | Register Lua callback function name |
| `obj:tick()` | Drive I/O and event dispatch |
| `obj:send(connid, data)` | Send data (server: specify connid; client: omit) |
| `obj:close()` | Close connection/server |
| `obj:close_connection(connid)` | Close single connection (server only) |
| `obj:get_events()` | Event history |
| `obj:get_last_data()` | Last received data |
| `obj:get_conn_count()` | Connection count |
| `obj:get_recv_count()` | Packet receive count |
| `obj:get_connid()` | Last connection ID (server only) |

---

## Timer

**File:** `timer/native_timer.h` · **Registration:** `RegisterTimerLibraryApi`

| Function | Args | Description |
|----------|------|-------------|
| `timer.set(delay_ms, func_name)` | 2 | One-shot timer; returns `timer_id` |
| `timer.del(timer_id)` | 1 | Delete pending timer |
| `timer.tick()` | 0 | Fire expired timers and heartbeat |
| `timer.set_heartbeat(interval_ms, func_name)` | 2 | Periodic heartbeat; auto-reschedules, overwrites previous |
| `timer.register_obj_methods(obj)` | 1 | Register `get_int`/`set_int`/`add_int` on a NativeObject for shared state |

**Callback signature:** `function cb(type, timer_id)` where `type == "timer"`

---

## Event

**File:** `event/native_event.h` · **Registration:** `RegisterEventLibraryApi`

| Function | Args | Description |
|----------|------|-------------|
| `event.on(event_name, func_name)` | 2 | Subscribe handler |
| `event.once(event_name, func_name)` | 2 | Subscribe once (auto-remove after fire) |
| `event.off(event_name, func_name)` | 2 | Unsubscribe handler |
| `event.emit(event_name, ...)` | vararg | Fire event; up to 4 args forwarded to handlers |
| `event.clear(event_name)` | 1 | Remove all handlers for event |
| `event.clear_all()` | 0 | Remove all handlers for all events |

> Re-entrancy safe: `emit` snapshots the handler list before iteration.

---

## Compress

**File:** `compress/native_compress.h` · **Registration:** `RegisterCompressLibraryApi`

| Function | Args | Description |
|----------|------|-------------|
| `compress.lz4_compress(data)` | 1 | LZ4 frame compression (embeds original size) |
| `compress.lz4_decompress(data)` | 1 | LZ4 decompression |
| `compress.zlib_compress(data, [level])` | 1-2 | zlib deflate, level 1-9, default 6 |
| `compress.zlib_decompress(data)` | 1 | zlib inflate |
| `compress.gzip_compress(data, [level])` | 1-2 | gzip compress, level 1-9, default 6 |
| `compress.gzip_decompress(data)` | 1 | gzip decompress |
| `compress.zstd_compress(data, [level])` | 1-2 | Zstandard, level 1-22, default 3 |
| `compress.zstd_decompress(data)` | 1 | Zstandard decompress |

---

## Crypto

**File:** `crypto/native_crypto.h` · **Registration:** `RegisterCryptoLibraryApi`

| Function | Args | Description |
|----------|------|-------------|
| `crypto.md5(data)` | 1 | MD5 hash → hex string |
| `crypto.sha1(data)` | 1 | SHA-1 hash → hex string |
| `crypto.sha256(data)` | 1 | SHA-256 hash → hex string |
| `crypto.hex_encode(data)` | 1 | Binary → hex string |
| `crypto.hex_decode(hex)` | 1 | Hex → binary |
| `crypto.base64_encode(data)` | 1 | Binary → base64 (RFC 4648) |
| `crypto.base64_decode(data)` | 1 | Base64 → binary |
| `crypto.aes_encrypt_ecb(data, key)` | 2 | AES-128-ECB encrypt (data 16-byte aligned) |
| `crypto.aes_decrypt_ecb(data, key)` | 2 | AES-128-ECB decrypt |
| `crypto.aes_encrypt_cbc(data, key, iv)` | 3 | AES-128-CBC encrypt (PKCS#7 padding) |
| `crypto.aes_decrypt_cbc(data, key, iv)` | 3 | AES-128-CBC decrypt |
| `crypto.aes_encrypt_ctr(data, key, iv)` | 3 | AES-128-CTR encrypt (stream, no padding) |
| `crypto.aes_decrypt_ctr(data, key, iv)` | 3 | AES-128-CTR decrypt |
| `crypto.rc4(key, data)` | 2 | RC4 stream cipher (encrypt = decrypt) |
| `crypto.blowfish_encrypt(key, data)` | 2 | Blowfish ECB encrypt |
| `crypto.blowfish_decrypt(key, data)` | 2 | Blowfish ECB decrypt |
| `crypto.des_encrypt(key, data)` | 2 | DES ECB encrypt (key ≥ 8 bytes) |
| `crypto.des_decrypt(key, data)` | 2 | DES ECB decrypt |
| `crypto.triple_des_encrypt(key, data)` | 2 | 3DES ECB encrypt (key ≥ 24 bytes) |
| `crypto.triple_des_decrypt(key, data)` | 2 | 3DES ECB decrypt |

---

## CSV

**File:** `csv/native_csv.h` · **Registration:** `RegisterCsvLibraryApi`

| Function | Args | Description |
|----------|------|-------------|
| `csv.decode(str, [sep])` | 1-2 | Parse CSV to table of rows; auto-converts numeric fields; default sep `,` |
| `csv.encode(rows, [sep])` | 1-2 | Encode table of rows to CSV; auto-quotes special fields; default sep `,` |

---

## JSON

**File:** `json/native_json.h` · **Registration:** `RegisterJsonLibraryApi`

| Function | Args | Description |
|----------|------|-------------|
| `json.encode(value)` | 1 | Lua value → JSON string; consecutive int keys 1..N → array; floats use `%.17g` |
| `json.decode(str)` | 1 | JSON string → Lua value; `null` → `nil` |

---

## MySQL

**Files:** `mysql/native_mysql.h`, `mysql/native_mysql_pool.h` · **Registration:** `RegisterMysqlLibraryApi`, `RegisterMysqlPoolApi`

**Config for `mysql.connect`:** `{host, port, user, password, db}`

**Config for `mysql_pool.create`:** `{host, port, user, password, db, pool_size, timeout_ms, heartbeat_ms, max_retries}`

| Function/Method | Description |
|----------|-------------|
| `mysql.connect(config, cb)` | Async connect; callback `function cb(err, conn)` |
| `mysql_pool.create(config)` | Create connection pool |
| `conn:query(sql, cb)` | Async query; callback `function cb(err, result)` |
| `conn:stmt_prepare(sql, cb)` | Prepare statement |
| `conn:stmt_execute(id, params, cb)` | Execute prepared statement |
| `conn:stmt_close(id)` | Close prepared statement |
| `conn:tick()` | Pump network events |
| `conn:close()` | Close connection |
| `pool:acquire()` | Get connection from pool |
| `pool:release(conn)` | Return connection to pool |
| `pool:tick()` | Drive heartbeat and reconnect |
| `pool:close()` | Close pool |
| `pool:stats()` | Returns `{total, healthy}` |

---

## SQLite

**File:** `sqlite/native_sqlite.h` · **Registration:** `RegisterSqliteLibraryApi`

| Function/Method | Description |
|----------|-------------|
| `sqlite.open(filename)` | Open/create database, returns db object |
| `db:exec(sql)` | Execute SQL; SELECT returns row table, non-SELECT returns nil |
| `db:prepare(sql)` | Returns prepared statement object |
| `db:last_insert_rowid()` | Last insert rowid |
| `db:changes()` | Rows affected by last statement |
| `db:close()` | Close database |
| `stmt:bind(...)` | Bind parameters (nil/int/float/string/bool) |
| `stmt:step()` | Execute one step; returns row table or nil |
| `stmt:reset()` | Reset for re-execution |
| `stmt:columns()` | Column names table |
| `stmt:close()` | Finalize statement |

> All operations are synchronous, based on SQLite3 amalgamation source.

---

## Serialize

**File:** `serialize/native_serialize.h` · **Registration:** `RegisterSerializeLibraryApi`

| Function | Args | Description |
|----------|------|-------------|
| `serialize.encode(value)` | 1 | Lua value → binary wire format |
| `serialize.decode(data)` | 1 | Binary wire format → Lua value |

**Encoding:** zigzag + varint for integers, little-endian 8-byte memcpy for floats, string deduplication (identical strings store varint reference ID from second occurrence), recursive table serialization.

**Supported types:** `nil`, `boolean`, integer, float, string (binary-safe), table (nested). Unsupported types in tables are skipped; unsupported top-level type errors.

---

## Protobuf

**File:** `protobuf/native_protobuf.h` · **Registration:** `RegisterProtobufLibraryApi`

| Function | Args | Description |
|----------|------|-------------|
| `protobuf.load(proto_text)` | 1 | Parse proto3 text, register all messages/enums |
| `protobuf.encode(name, table)` | 2 | Lua table → protobuf binary |
| `protobuf.decode(name, data)` | 2 | Protobuf binary → Lua table |
| `protobuf.types()` | 0 | Registered message names |
| `protobuf.fields(name)` | 1 | Field info `{name, number, type, type_name, label}` |

**Supported proto3 features:** message (nested), enum, map\<K,V\>, oneof, repeated (packed by default), optional (explicit presence), all 18 scalar types, import (multi-file).

**Wire format:** tag = field_number << 3 | wire_type; varint for integers (zigzag for sint); little-endian memcpy for floats; length-prefixed string/bytes/message; packed repeated scalars by default.

---

## NativeObject (Lua-side API)

**File:** `object/native_object.h` · **Registration:** `RegisterNativeObjectApi`

| Function | Args | Description |
|----------|------|-------------|
| `new_native_group()` | 0 | Create group arena, returns `group_id` |
| `new_native_obj(group_id, type, id)` | 3 | Create object in group |
| `get_native_obj(type, id)` | 2 | Find object by type+id |
| `del_native_group(group_id)` | 1 | Destroy all objects in group, returns count |
| `new_global_obj(key, type)` | 2 | Create global object (string key indexed) |
| `get_global_obj(key)` | 1 | Find global object |
| `del_global_obj(key)` | 1 | Destroy global object |

**NativeObject C++ API** (for host-side binding):

| Method | Description |
|--------|-------------|
| `RegisterMethod(name, lambda)` | Bind C++ function as Lua-callable method |
| `GetInt/SetInt/GetFloat/SetFloat/GetBool/SetBool/GetString/SetString` | Property accessors |
| `GetGroup/GetType/GetId` | Identity accessors |
| `DestroyGroup(group_id)` | Batch-destroy all objects in group |
| `SetFinalizer(fn)` | Set cleanup callback |

---

## Shared Utilities

**File:** `native_common.h`

| Function | Description |
|----------|-------------|
| `ThrowBadArgument(argno, fname, expected)` | Throw standardized "bad argument" error |
| `CheckNumberArg(a, argno, fname)` | Reject non-number where number expected |
| `CheckIntegerArg(a, argno, fname)` | Lua 5.4-aligned integer check; Int passes, Float must be lossless integer |
| `CheckStringArg(a, argno, fname)` | Reject non-string where string expected |
| `MakeIteratorClosure(state, fn, iter_state)` | Construct iterator closure for `pairs`/`ipairs`/`gmatch`/`file:lines` |
