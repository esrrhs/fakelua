[中文](README.zh.md) | English

# FakeLua Native Libraries

Detailed API reference for all built-in native libraries. Each module lives in its own subdirectory under `src/native/` with a `.h` and `.cpp` file pair.

> For a high-level overview, see the [main README](../README.md).

## Module List

| Module | Directory | Description |
|--------|-----------|-------------|
| basic | `basic/` | Global functions: `print`, `type`, `tostring`, `tonumber`, `select`, `error`, `assert`, `pcall`, `xpcall`, `next`, `pairs`, `ipairs`, `collectgarbage` |
| math | `math/` | Math functions: arithmetic, trigonometry, exponential/logarithm, random, constants, special functions |
| table | `table/` | Table operations: `insert`, `remove`, `concat`, `sort`, `pack`, `unpack`, `move`, `create` |
| string | `string/` | String operations: substring, case, trim/split/join/replace, pattern matching (ECMAScript regex), formatting, binary pack/unpack, serialization |
| os | `os/` | OS interface: time, date, environment, file operations, process execution (UTF-8 paths via Boost.Nowide on Windows) |
| utf8 | `utf8/` | UTF-8 encoding/decoding: `char`, `codepoint`, `codes`, `len`, `offset` |
| io | `io/` | File I/O: open, close, read, write, seek, popen, standard streams |
| net | `net/` | TCP/UDP networking: server/client with framed protocols, custom parsers, async event dispatch |
| http | `http/` | HTTP/1.1 client and server (Boost.Beast), driven by `runtime.tick()` |
| url | `url/` | URL parse/format and percent-encoding (Boost.URL) |
| timer | `timer/` | Timers: one-shot, periodic heartbeat, driven by `runtime.tick()` |
| runtime | `runtime/` | Unified event loop pump: `runtime.tick()` drives every module that needs periodic progress |
| event | `event/` | Pub/sub event system: `on`, `once`, `off`, `emit`, `clear`, `clear_all` |
| random | `random/` | Seeded RNG (PCG-32): `int`, `float`, `dice`, `chance`, `weighted`, `get_state`, `set_state` |
| container | `container/` | Persistent deque / vector / small_vector / list / ordered map / set (Boost.Container), NativeObject-backed |
| compress | `compress/` | Compression: LZ4, zlib, gzip, Zstd |
| crypto | `crypto/` | Cryptography: MD5/SHA1/SHA256, hex/base64, UUID, CRC-32, xxHash-64, AES/RC4/Blowfish/DES/3DES |
| csv | `csv/` | CSV decode/encode |
| json | `json/` | JSON encode/decode |
| mysql | `mysql/` | Async MySQL client: direct connect + connection pool |
| redis | `redis/` | Async Redis client (Boost.Redis) |
| sqlite | `sqlite/` | SQLite3 wrapper: exec, prepared statements, synchronous |
| process | `process/` | Subprocess spawn (Boost.Process v2); does not replace `os.execute` |
| serialize | `serialize/` | Binary serialization with zigzag+varint encoding and string deduplication |
| protobuf | `protobuf/` | Runtime .proto parsing, standard protobuf3 wire encode/decode |
| object | `object/` | NativeObject Lua-side API: group management, object creation/lookup |

---

## Threading Model

A `State` is a single-threaded entity: only one thread may touch it at a time, and concurrency
is achieved by giving each thread its own `State`.

The native layer is built on that premise: **all mutable state hangs off the `State`**. A module
reaches its own private state through `State::GetModuleState<T>()`, which creates it on first
access and destroys it with the `State`. No container is shared between States, so no locking is
needed. Concretely:

- Native objects, groups, global objects and id allocation are per-`State`, reached via
  `State::GetNativeObjectManager()` (C++ callers can also use the free function
  `GetNativeObjectManager(State *)`, since `State` is an opaque type outside the library);
- Timers, event listeners, the net/mysql/sqlite/io object tables and the protobuf .proto schema
  registry are all per-`State`, so nothing registered in one `State` leaks into another;
- Reusable scratch buffers belong to whatever they serve: the linear staging areas used when
  unpacking live on the `CircularBuffer` (`HeaderScratch` / `PayloadScratch`). WebSocket
  uses Boost.Beast, which owns handshake and masking on the connection. Random number
  generators that are only hit occasionally (temp file names) are plain locals.

The JIT error boundary chain (`jit_error_boundary.h`) lives on `State` too: the top of the chain
is `State::GetJitErrorBoundary()`, while the boundary objects themselves sit on the C++ stack.
`RunWithJitErrorBoundary`, `GuardJitEntry` and `InJitFrame` all take a `State`, which is why
`inter::DispatchCall` carries a `State` parameter as well. A `State` is owned by exactly one
thread and its scripts only ever run on that thread's stack, so the chain top is naturally
per-State.

Logging also takes a `State`: every `LOG_*` macro and the JIT helper `FakeluaLogLua` have
`State *` as the first argument. Level and log file are per-`State` (`StateConfig::log_level` /
`log_file`, or `log.set_level` / `log.set_file` at runtime). An empty `log_file` means console
only; `s == nullptr` (for example `ThrowFakeluaException`) is treated as Info and console only.
Console output is still serialized by a process-wide lock because stdout/stderr are shared.

There are no `thread_local` variables left.

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
| `math.erf(x)` | 1 | Error function (Boost.Math) |
| `math.erfc(x)` | 1 | Complementary error function |
| `math.gamma(x)` | 1 | Gamma function Γ(x) |
| `math.lgamma(x)` | 1 | Log-gamma ln Γ(x) |
| `math.clamp(x, lo, hi)` | 3 | Clamp `x` to `[lo, hi]` (Boost.Algorithm; all-integer args stay integer) |

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
| `string.trim(s)` | 1 | Strip leading/trailing whitespace (Boost.Algorithm, C locale) |
| `string.trim_left(s)` / `string.trim_right(s)` | 1 | Strip leading or trailing whitespace only |
| `string.split(s, sep)` | 2 | Split on separator string (multi-char OK; empty parts kept) |
| `string.join(tbl, sep)` | 2 | Join array elements with `sep` (inverse of `split`; empty `sep` concatenates) |
| `string.starts_with(s, prefix)` | 2 | Whether `s` begins with `prefix` |
| `string.ends_with(s, suffix)` | 2 | Whether `s` ends with `suffix` |
| `string.contains(s, needle)` | 2 | Whether `s` contains `needle` as a substring |
| `string.replace(s, from, to)` | 3 | Literal replace-all (`from` must be non-empty) |
| `string.iequals(a, b)` | 2 | Case-insensitive equality (C locale / ASCII) |
| `string.icontains(s, needle)` | 2 | Case-insensitive substring test (C locale / ASCII) |
| `string.istarts_with(s, prefix)` | 2 | Case-insensitive `starts_with` (C locale / ASCII) |
| `string.iends_with(s, suffix)` | 2 | Case-insensitive `ends_with` (C locale / ASCII) |
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
| `string.pack(fmt, ...)` | vararg | Binary pack (Lua 5.3 format; `<`/`>`/`=` via Boost.Endian) |
| `string.packsize(fmt)` | 1 | Compute packed size for format |
| `string.unpack(fmt, s, [pos])` | vararg | Binary unpack (Lua 5.3 format; `<`/`>`/`=` via Boost.Endian) |

> ⚠️ `string.find`/`match`/`gmatch`/`gsub` use **ECMAScript regex** (`boost::regex::ECMAScript`), not Lua patterns. See [Regex Matching](../README.md#regex-matching-uses-ecmascript-syntax-not-lua-patterns) in the main README.

---

## OS

**File:** `os/native_os.h` · **Registration:** `RegisterOsLibraryApi`

| Function | Args | Description |
|----------|------|-------------|
| `os.clock()` | 0 | CPU time in seconds |
| `os.date([fmt, [time]])` | vararg | Formatted date/time string; `"*t"` returns table `{year, month, day, hour, min, sec, wday, yday, isdst}` |
| `os.difftime(t2, t1)` | 2 | Difference between two timestamps |
| `os.execute([cmd])` | vararg | Execute shell command; returns `(status_bool_or_nil, "exit"|"signal"|"error", code)` triple. Uses Boost.Nowide `system` on Windows so the command string is UTF-8. |
| `os.exit([code, [close]])` | vararg | Terminate process |
| `os.getenv(name)` | 1 | Get environment variable |
| `os.remove(filename)` | 1 | Delete file or empty directory (Boost.Filesystem); `true` or `nil` |
| `os.rename(old, new)` | 2 | Rename/move (Boost.Filesystem); `true` or `nil` |
| `os.setlocale(locale, [cat])` | vararg | Set/query locale |
| `os.time([table])` | vararg | Current time or timestamp from table |
| `os.tmpname()` | 0 | Create a unique empty temp file and return its path (Boost.Filesystem) |
| `os.sleep(ms)` | 1 | Sleep milliseconds (FakeLua extension; `os.sleep(1)` is 1 ms) |
| `os.exists(path)` | 1 | Whether path exists |
| `os.isfile(path)` / `os.isdir(path)` | 1 | Regular file / directory |
| `os.filesize(path)` | 1 | Byte size, or `nil` |
| `os.mtime(path)` | 1 | Last write time as Unix timestamp, or `nil` |
| `os.mkdir(path)` | 1 | Create directory and parents |
| `os.remove_all(path)` | 1 | Recursive delete; returns entries removed, or `nil` |
| `os.copy(from, to)` | 2 | Copy file or directory tree (overwrite) |
| `os.listdir(path)` | 1 | Sorted directory names (1-based table), or `nil` |
| `os.getcwd()` / `os.chdir(path)` | 0/1 | Get/set working directory |
| `os.absolute(path)` / `os.canonical(path)` | 1 | Absolute / weakly-canonical path |
| `os.join(...)` | vararg | Join path segments |
| `os.dirname(path)` / `os.basename(path)` / `os.extension(path)` | 1 | Parent, filename, extension (including `.`) |

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

On Windows, `io.open` / `loadfile` / `dofile` / `os.getenv` / `os.tmpname` / Boost.Filesystem path APIs go through Boost.Nowide so Lua strings are treated as UTF-8. POSIX is unchanged.

---

## Net

**File:** `net/native_net.h` · **Registration:** `RegisterNetLibraryApi`

**Config table fields:** `ip`, `port`, `maxconn`, `backlog`, `nodelay`, `keepalive`, `framer`, `parser`, `fixed_len`, `ws_path`, `ws_host`, `ws_origin`

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
| `websocket` / `ws` | RFC 6455 WebSocket (text frames, Boost.Beast) |

**Custom parser:** `parser = "Package.func"` (Lua) or `custom_parser_fn`/`custom_encoder_fn` (C++ `NetConfig`)

**WebSocket extras:** `ws_path` (default `"/"`), `ws_host` (client Host, default `ip:port`), `ws_origin` (optional). TLS (`wss`): `tls=true`, server `cert`/`key`, client `tls_verify` (default true) and optional `tls_ca`.

| Function/Method | Description |
|----------|-------------|
| `net.server(config)` | Create TCP server |
| `net.client(config)` | Create TCP client |
| `net.ws_server(config)` | Create WebSocket server (same as `framer="websocket"`) |
| `net.ws_client(config)` | Create WebSocket client |
| `net.udp_server(config)` | Bind a UDP socket (`ip`, `port`; `port=0` is ephemeral). `send(data, ip, port)` |
| `net.udp_client(config)` | Connected UDP client (`ip`/`port` are the peer). `send(data)` |
| `obj:dispatch(func_name)` | Register Lua callback function name |
| `obj:send(connid, data)` | Send data (server: specify connid; client: omit) |
| `obj:close()` | Close connection/server |
| `obj:close_connection(connid)` | Close single connection (server only) |
| `obj:get_events()` | Event history |
| `obj:get_last_data()` | Last received data |
| `obj:get_conn_count()` | Connection count |
| `obj:get_recv_count()` | Packet receive count |
| `obj:get_connid()` | Last connection ID (server only) |
| `udp:get_port()` | Bound UDP port |
| `udp:get_peer()` | Last datagram peer as `ip, port` |

---

## HTTP

**File:** `http/native_http.h` · **Registration:** `RegisterHttpLibraryApi`

HTTP/1.1 client and server on Boost.Beast, sharing the per-State Asio `io_context`. URLs are parsed with Boost.URL. `https://` uses TLS (OpenSSL); `http.get`/`http.post` verify the certificate by default. Drive completions with `runtime.tick()`.

**`http.request` config:** `{method, url, headers, body, timeout_ms, version, tls_verify, tls_ca}`

**`http.server` config:** `{ip, port, backlog, timeout_ms, tls, cert, key}` (`port = 0` binds an ephemeral port; read `srv.port` or `srv:get_port()`). `tls=true` requires `cert` and `key` PEM paths.

| Function/Method | Description |
|----------|-------------|
| `http.request(config, cb)` | Async request; callback `function cb(req, err, resp)` |
| `http.get(url, cb)` | GET helper |
| `http.post(url, body, cb)` | POST helper (`Content-Type: text/plain`) |
| `http.server(config)` | Listen; returns server object |
| `srv:dispatch(func_name)` | Request callback `function cb(typ, connid, req)` (`typ == "request"`). Returning a response table/string replies immediately. |
| `srv:reply(connid, resp)` | Send `{status, reason, headers, body}` (or a string body) |
| `srv:get_port()` | Bound TCP port |
| `srv:close()` / `req:close()` | Close server or cancel in-flight request |

`resp` / `req` tables: `status`, `reason`, `headers`, `body`; incoming requests also have `method`, `target`, `path`, `query`, `version`.

---

## URL

**File:** `url/native_url.h` · **Registration:** `RegisterUrlLibraryApi`

| Function | Args | Description |
|----------|------|-------------|
| `url.parse(str)` | 1 | URI reference → `{scheme, user, password, host, port, path, query, params, fragment, href}` |
| `url.format(tbl)` | 1 | Table → URL string |
| `url.encode(str)` | 1 | Percent-encode (unreserved charset) |
| `url.decode(str)` | 1 | Percent-decode (`+` as space) |
| `url.encode_query(tbl)` | 1 | Table → `application/x-www-form-urlencoded` |
| `url.decode_query(str)` | 1 | Query string → table |

---

## Timer

**File:** `timer/native_timer.h` · **Registration:** `RegisterTimerLibraryApi`

| Function | Args | Description |
|----------|------|-------------|
| `timer.set(delay_ms, func_name)` | 2 | One-shot timer; returns `timer_id` |
| `timer.del(timer_id)` | 1 | Delete pending timer |
| `timer.set_heartbeat(interval_ms, func_name)` | 2 | Periodic heartbeat; auto-reschedules, overwrites previous |
| `timer.register_obj_methods(obj)` | 1 | Register `get_int`/`set_int`/`add_int` on a NativeObject for shared state |

**Callback signature:** `function cb(type, timer_id)` where `type == "timer"`

---

## Runtime

**File:** `runtime/native_runtime.h` · **Registration:** `RegisterRuntimeLibraryApi`

| Function | Args | Description |
|----------|------|-------------|
| `runtime.tick()` | 0 | Drive every native module on this State that needs periodic progress |

This is the only pump a script needs: it calls `timer::TickAll`, `net::TickAll`, `http::TickAll`,
`mysql::TickAll` and `redis::TickAll` in that fixed order, the same way `FakeluaDeleteState` dispatches
`OnStateDeleted` to each module. Each module walks its own per-State object list, so
servers, clients, connections and pools have no per-object `tick()` method — call this
from your main loop instead.

Timers go first so callbacks that come due can be picked up by the I/O dispatch behind
them. Within MySQL, pools are driven before connections so heartbeat and reconnect have
run before a script acquires a connection this round.

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

## Random

**File:** `random/native_random.h` · **Registration:** `RegisterRandomLibraryApi`

PCG-32 algorithm: 64-bit state, 32-bit output, period 2^64. Each `random.new(seed)` creates an independent RNG stream backed by a NativeObject. Different seeds produce different sequences; same seed produces identical sequences. State serialized as hex string to avoid 32-bit truncation.

| Function | Args | Description |
|----------|------|-------------|
| `random.new(seed)` | 1 | Create RNG instance with given seed (integer) |
| `rng:int(min, max)` | 2 | Uniform integer in [min, max] (inclusive) |
| `rng:float(min, max)` | 2 | Uniform float in [min, max) |
| `rng:dice(count, sides)` | 2 | Sum of `count` dice, each [1, sides] |
| `rng:chance(prob)` | 1 | `true` with probability `prob` (0.0–1.0) |
| `rng:weighted(weights)` | 1 | Pick 1-based index from weight table; zero-weight entries never selected |
| `rng:get_state()` | 0 | Get 64-bit internal state as hex string (e.g. `"0x1234567890ABCDEF"`) |
| `rng:set_state(hex_str)` | 1 | Restore 64-bit internal state from hex string |

> **Multi-stream:** Create separate RNG instances for independent streams (e.g. combat RNG, loot RNG, event RNG). Using a single global RNG causes cross-system correlation — consuming randoms in one system shifts the sequence for all others.

> **Save/Load:** Use `get_state()` / `set_state()` with hex strings for serialization. The hex string is JSON-safe and preserves the full 64-bit state without precision loss.

---

## Container

**File:** `container/native_container.h` · **Registration:** `RegisterContainerLibraryApi`

Boost.Container-backed structures stored on a NativeObject (C++ heap). They survive arena reset; Lua tables in the same slot do not. Keys/values are `nil` / boolean / number / string (or another NativeObject as a sequence/map value). Plain Lua tables and functions are rejected. Map/set keys are ordered: nil < bool < int < float < string, then by value. Integer-valued floats are stored as ints.

| Function | Args | Description |
|----------|------|-------------|
| `container.deque()` | 0 | Create a double-ended queue (`boost::container::deque`) |
| `d:push_back(v)` / `d:push_front(v)` | 1 | Insert at back/front |
| `d:pop_back()` / `d:pop_front()` | 0 | Remove and return; `nil` if empty |
| `d:front()` / `d:back()` | 0 | Peek; `nil` if empty |
| `d:at(i)` | 1 | 1-based get; `nil` if out of range |
| `d:set(i, v)` | 2 | 1-based replace; errors if out of range |
| `d:to_table()` | 0 | Copy to a 1-based Lua array |
| `container.vector()` | 0 | Create a vector (`boost::container::vector`); back insert only |
| `v:push_back(v)` / `v:pop_back()` | 1/0 | Append / remove last (`nil` if empty) |
| `v:front()` / `v:back()` / `v:at(i)` / `v:set(i, v)` / `v:to_table()` | | Same 1-based indexing as deque |
| `container.small_vector()` | 0 | Like vector, inline storage for the first 8 elements (`boost::container::small_vector`) |
| `container.list()` | 0 | Doubly-linked list (`boost::container::list`); `at`/`set` are O(n) |
| `l:push_front(v)` / `l:pop_front()` | 1/0 | Head insert / remove, in addition to the vector-style back methods |
| `container.map()` | 0 | Create an ordered map (`boost::container::flat_map`) |
| `m:set(k, v)` / `m:get(k)` / `m:has(k)` / `m:erase(k)` | 1-2 | Insert/overwrite, lookup (`nil` if missing), membership, erase (bool) |
| `m:keys()` / `m:to_table()` | 0 | Sorted keys array, or Lua table of pairs |
| `container.set()` | 0 | Create an ordered unique set (`boost::container::flat_set`) |
| `s:insert(v)` / `s:has(v)` / `s:erase(v)` | 1 | Insert (bool if new), membership, erase (bool) |
| `s:values()` | 0 | Sorted values as a 1-based Lua array |
| `*:size()` / `*:empty()` / `*:clear()` / `*:close()` | 0 | Shared: length, emptiness, drop contents, destroy |

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
| `crypto.hex_encode(data)` | 1 | Binary → hex string (lowercase, Boost.Algorithm) |
| `crypto.hex_decode(hex)` | 1 | Hex → binary (Boost.Algorithm `unhex`) |
| `crypto.base64_encode(data)` | 1 | Binary → base64 (RFC 4648) |
| `crypto.base64_decode(data)` | 1 | Base64 → binary |
| `crypto.uuid()` | 0 | RFC 4122 v4 UUID string |
| `crypto.crc32(data)` | 1 | CRC-32/ISO-HDLC (PKZIP) → unsigned 32-bit integer |
| `crypto.xxhash(data)` | 1 | xxHash-64 → 16-char lowercase hex (Boost.Hash2; not a password hash). MD5/SHA stay on OpenSSL. |
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

**Config for `mysql.connect`:** `{host, port, user, password, db, timeout_ms, ssl, ssl_ca}`

**Config for `mysql_pool.create`:** `{host, port, user, password, db, pool_size, timeout_ms, heartbeat_ms, max_retries, ssl, ssl_ca}`

`ssl`: omit/`false`/`"disable"` keeps plaintext (default). `true`/`"require"` demands TLS. `"enable"` uses TLS when the server offers it. Optional `ssl_ca` PEM enables certificate verification.

| Function/Method | Description |
|----------|-------------|
| `mysql.connect(config, cb)` | Async connect; callback `function cb(err, conn)` |
| `mysql_pool.create(config)` | Create connection pool |
| `conn:query(sql, cb)` | Async query; callback `function cb(err, result)` |
| `conn:stmt_prepare(sql, cb)` | Prepare statement |
| `conn:stmt_execute(id, params, cb)` | Execute prepared statement |
| `conn:stmt_close(id)` | Close prepared statement |
| `conn:close()` | Close connection |
| `pool:acquire()` | Get connection from pool |
| `pool:release(conn)` | Return connection to pool |
| `pool:close()` | Close pool |
| `pool:stats()` | Returns `{total, healthy}` |

---

## Redis

**File:** `redis/native_redis.h` · **Registration:** `RegisterRedisLibraryApi`

Async client on Boost.Redis. Reconnect and health-checks are off so a failed connect completes once. Drive with `runtime.tick()`.

**`redis.connect` config:** `{host, port, user, password, db, timeout_ms}`

| Function/Method | Description |
|----------|-------------|
| `redis.connect(config, cb)` | Async connect; callback `function cb(conn, err, success)` |
| `conn:command(argv, cb)` | Run a command; `argv` is a 1-based string array, callback `function cb(conn, err, result)` |
| `conn:close()` | Close connection |

Connect does not send `HELLO 3`, so Redis 4+ works; AUTH/SELECT are issued when `password` / `db` are set. Arrays become Lua arrays; RESP3 maps become tables; bulk/simple strings stay strings. After an optional `HELLO 3`, `HGETALL` is a map.

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

## Process

**File:** `process/native_process.h` · **Registration:** `RegisterProcessLibraryApi`

Boost.Process v2. This does **not** replace `os.execute` (that still returns Lua's `(status, how, code)` triple).

| Function | Args | Description |
|----------|------|-------------|
| `process.run(argv [, opts])` | 1-2 | Spawn `argv[1]` with the rest as arguments. Returns `stdout, stderr, exit_code`. `opts`: `{stdin, cwd, env, timeout_ms}` |

`env` is a string→string table merged into the current environment. `timeout_ms` kills the child (non-zero exit). Missing executables throw.

---

## Serialize

**File:** `serialize/native_serialize.h` · **Registration:** `RegisterSerializeLibraryApi`

| Function | Args | Description |
|----------|------|-------------|
| `serialize.encode(value)` | 1 | Lua value → compact binary wire format |
| `serialize.decode(data)` | 1 | Compact binary wire format → Lua value |
| `serialize.text_encode(value)` | 1 | Lua value → Boost.Serialization text archive |
| `serialize.text_decode(data)` | 1 | Text archive → Lua value |
| `serialize.xml_encode(value)` | 1 | Lua value → Boost.Serialization XML archive |
| `serialize.xml_decode(data)` | 1 | XML archive → Lua value |

**Encoding:** `encode`/`decode` use zigzag + varint for integers, little-endian 8-byte memcpy for floats, string deduplication (identical strings store varint reference ID from second occurrence), recursive table serialization. `text_*` / `xml_*` use Boost.Serialization archives (readable, not interchangeable with the compact wire).

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
