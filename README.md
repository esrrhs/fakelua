# FakeLua
[<img src="https://img.shields.io/github/license/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/languages/top/esrrhs/fakelua">](https://github.com/esrrhs/fakelua)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build.yml?branch=master&label=Linux">](https://github.com/esrrhs/fakelua/actions/workflows/build.yml)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build_with_macos.yml?branch=master&label=macOS">](https://github.com/esrrhs/fakelua/actions/workflows/build_with_macos.yml)
[<img src="https://img.shields.io/github/actions/workflow/status/esrrhs/fakelua/build_with_windows.yml?branch=master&label=Windows">](https://github.com/esrrhs/fakelua/actions/workflows/build_with_windows.yml)
[![codecov](https://codecov.io/gh/esrrhs/fakelua/graph/badge.svg?token=9ZCUH1Q632)](https://codecov.io/gh/esrrhs/fakelua)

[中文](README.zh.md) | English

FakeLua is an embeddable Lua-subset compilation engine: it compiles Lua scripts into C code and dynamically compiles them into native machine code via the GCC backend for execution. It provides a C++23 interface with high-performance interop between scripts and native code.

## Design Philosophy & Memory Model

FakeLua was designed to address the **throughput jitter and memory bloat** caused by garbage collection in traditional scripting languages (standard Lua/LuaJIT) when used in **high-performance game servers** or similar real-time systems.

### 1. Scripting as High-Cohesion "Business Glue"
In a typical real-time high-performance server architecture:
*   **State & data reside in C++**: Core data structures (player state, world maps, monster attributes, physics engine) are all stored in efficient, compact, type-safe C++ on the host side.
*   **Stateless/shallow-state Lua logic layer**: Lua is used only for pure logic processing and business orchestration — reading C++ data and invoking C++ functions. Scripts should not retain large-scale data objects long-term.

### 2. Memory: Ultra-Fast Arena Pool + Frame Reset
To support this positioning, FakeLua **does not implement a complex dynamic garbage collector** (tri-color marking, generational GC, etc.). Instead, it uses an extremely efficient **Arena memory pool (Bump Allocator)**:
*   **Bump allocation ($O(1)$)**: When creating temporary variables (Table, String, Multi, etc.), FakeLua simply moves an offset pointer within a pre-allocated contiguous memory block. Allocation is nearly as fast as native stack allocation, without `malloc` fragmentation or overhead.
*   **Instant cleanup ($O(1)$)**: At the end of each frame or request processing, `State::Reset()` is called. It invokes destructors in reverse order, but instead of freeing individual blocks, the pool offset pointer is simply reset to zero. There is no complex object graph traversal, no system-level `free` cost or defragmentation — cleanup is instantaneous.

This design allows FakeLua to fully eliminate GC pause impact on frame rates while maintaining JIT native execution speed, keeping memory overhead at a completely predictable, extremely low level.

## Core Features

### Dual JIT Backends

Supports two JIT modes with a seamless API switch:

- **JIT_GCC**: Invokes system GCC (`-O3`) to generate high-quality native code. This is the primary backend for production use.
- **JIT_TCC**: Embeds TinyCC for extremely fast compilation. Primarily used for development, debugging, and testing (TCC source is automatically fetched during CMake configuration — no system installation needed).

```cpp
int ret = 0;
// Same Call API, switching between JIT_GCC and JIT_TCC on demand
Call(s, JIT_GCC, "add", ret, 10, 20); // Production: GCC backend (-O3 high performance)
Call(s, JIT_TCC, "add", ret, 10, 20); // Development: TCC backend (ultra-fast compilation)
```

### Numeric Specialization

The compiler automatically performs type inference and specialization for function math parameters:

1. [TypeInferencer](src/compile/type_inferencer.h) runs iterative fixed-point inference on each top-level function (leave-one-out) to identify parameters that truly participate in arithmetic (math params).
2. [CGen](src/compile/c_gen.h) generates $2^k$ specializations (`int64_t` / `double` combinations) plus a runtime entry dispatcher that routes to the appropriate specialization based on actual argument types.
3. Specialized bodies use native C types (`int64_t`/`double`) for arithmetic and generate native C `bool` for comparisons, completely eliminating boxing overhead on hot paths.

```lua
-- Example Lua function: recursive Fibonacci
function fib(n)
    if n <= 1 then return n end
    return fib(n - 1) + fib(n - 2)
end
```

Auto-generated specialized C code:

```c
// 1. Numeric specialization: params/return promoted to native int64_t, no CVar boxing
static int64_t fib_spec_0(int64_t n) {
    if (n <= 1) {
        return n;
    }
    return fib_spec_0(n - 1) + fib_spec_0(n - 2);
}

// 2. Generic entry dispatcher: fast type check, zero-overhead routing to native C specialization
static CVar fib_dispatcher(CVar n_var) {
    if (LIKELY(n_var.type_ == VAR_INT)) {
        return (CVar){.type_ = VAR_INT, .data_.i = fib_spec_0(n_var.data_.i)};
    }
    // ... dynamic dispatch to double specialization or generic CVar path
}
```

With recursive Fibonacci (n=32) as an example, the GCC backend is **43.5x** faster than Lua 5.4, and the TCC backend is **11.2x** faster (see [benchmark/README.md](benchmark/README.md)).

### Table Struct Specialization

If a Table constructor can statically infer all its keys at compile time (string literals, explicit/implicit integer indices, booleans, floats), the compiler specializes it as a C struct:

1. **Struct layout generation**: The compiler dynamically generates a C struct layout at compile time, with each specialized key mapped to a fixed-offset member.
2. **Initialization & deduplication**: Constructor initialization fills the JIT specialized struct in a single pass (following Lua's left-to-right order) and checks for duplicate keys at compile time.
3. **Ultra-fast pointer-offset access**: For specialized key reads/writes, pointer offset macros (`FL_SPEC`/`FL_SET_SPEC`) are used directly, completely avoiding hash lookups and key comparisons.
4. **Dynamic fallback**: If the key used for read/write is a dynamic variable, it falls back to runtime dynamic dispatch via registered `spec_get` / `spec_set` function pointers.

```lua
-- Example Lua code: defining and accessing Table fields
local point = { x = 10, y = 20 }
point.x = point.x + 5
```

Auto-generated specialized C struct and pointer-offset access:

```c
// 1. Compile-time key layout inference, auto-generate C struct definition
typedef struct Table_Spec_1 {
    CVar x;
    CVar y;
} Table_Spec_1;

// 2. On Table initialization, bind specialized struct layout and spec accessors
SET_TABLE_SPEC(point, Table_Spec_1, spec_get_fn, spec_set_fn, 2);
FL_SET_SPEC(Table_Spec_1, point, x, 0, (CVar){.type_ = VAR_INT, .data_.i = 10});
FL_SET_SPEC(Table_Spec_1, point, y, 1, (CVar){.type_ = VAR_INT, .data_.i = 20});

// 3. Field access converted to ultra-fast pointer member offsets (no hash table lookup)
FL_SPEC(Table_Spec_1, point, x) = NativeAdd(FL_SPEC(Table_Spec_1, point, x), (CVar){.type_ = VAR_INT, .data_.i = 5});
```

### CVar: ABI-Safe Cross-Boundary Value Type

```cpp
struct CVar {
    int type_ = 0;
    int flag_ = 0;
    union cvar_data {
        bool b;
        int64_t i;
        double f;
        VarString *s;
        VarTable *t;
        VarMulti *m;
    };
    cvar_data data_{};
};
static_assert(std::is_standard_layout_v<CVar>);
static_assert(std::is_trivially_copyable_v<CVar>);
```

CVar is the sole value carrier between JIT code and the C++ host, enforced as standard-layout (POD) to guarantee ABI compatibility across platforms including arm64.

```cpp
// Conversion between native C++ types and JIT ABI-safe carrier CVar
CVar v_int = inter::NativeToFakelua(s, 42);
int native_int = inter::FakeluaToNative<int>(s, v_int);
```

### VarInterface: Extensible Complex Type Bridge

VarInterface is the abstract interface between Lua tables and the host. Hosts can implement their own version to plug into the existing object system. A SimpleVarImpl is included out of the box.

```cpp
class CustomVar : public VarInterface { /* inherit and extend with custom table implementation */ };

// Register factory function so FakeLua tables are automatically constructed as CustomVar
SetVarInterfaceNewFunc(s, []() { return new CustomVar(); });
```

### NativeObject: Native Object Bridge & Group Arena

Provides high-performance host C++ native object mapping:

- **Clean host public API**: Hides underlying storage details (Pimpl) in the SDK header, exposing only necessary property accessors (`GetInt`/`SetInt`/`GetFloat`/`SetObject`, etc.) and iteration methods.
- **Group-granularity batch release**: Individual allocation/deallocation is prohibited. All NativeObjects are created within a designated `group_id` pool (`NativeObjectManager::Instance().Create(group_id, ...)`), and batch-released via `DestroyGroup(group_id)` after request/frame processing — fully aligned with FakeLua's GC-free, Arena-reset design philosophy.
- **Global objects (string key indexing)**: For persistent singletons like global managers or counters that don't belong to any group (similar to Lua `_G.xxx`), a group-independent global object API is provided: `new_global_obj(key, type)` to create, `get_global_obj(key)` to find, `del_global_obj(key)` to destroy individually. Global objects have `group_id == 0` and are not affected by `DestroyGroup`.
- **C++ native member method binding**: Supports binding C++ functions/Lambdas as methods directly on NativeObjects via `RegisterMethod`. Lua can call them with colon syntax `obj:method(args...)`.
- **C++ function auto-boxing**: C++ native callbacks can return `NativeObject*` pointers directly, and `fakelua.h`'s `inter::NativeToFakelua` automatically boxes them into Lua-recognizable objects.

#### C++ Member Method Binding & Lua Interaction Example

```cpp
// 1. C++ host side: register native member methods on a NativeObject instance
player->RegisterMethod("take_damage", [](NativeObject *self, State *s, CVar *args, int n) -> CVar {
    int64_t dmg = inter::FakeluaToNative<int64_t>(s, inter::GetNativeArg(s, args, n, 0));
    self->SetInt("hp", self->GetInt("hp") - dmg);
    return inter::NativeToFakeluaNil(s);
});

player->RegisterMethod("is_alive", [](NativeObject *self, State *s, CVar *args, int n) -> CVar {
    return inter::NativeToFakeluaBool(s, self->GetInt("hp") > 0);
});
```

```lua
-- 2. Lua side: call bound C++ member methods with colon syntax
player:take_damage(30) -- invoke C++ callback, deduct hp

if player:is_alive() then
    print("Player is still alive, current HP:", player.hp)
end
```

### Package Module Management (Zero-Require Modules)

FakeLua provides a unique `package "ModuleName"` modular isolation and zero-`require` cross-module invocation capability:

- **Module package definition**: Declare module membership at the top of a script via `package "PackageName"`. Top-level exported functions in the current file are automatically bound to that package's namespace (e.g., `Player.AddItem`).
- **Zero-require cross-module calls**: No explicit `require` needed to load dependent files. As long as the relevant packages have been compiled and loaded into the same `State`, cross-module calls (e.g., `Player.AddItem(...)`) are automatically bound via dynamic routing.

#### Modular Code Example

```lua
-- player.lua
package "Player"

local BASE_BONUS = 1 -- package-private variable

function AddItem(id, num) -- exported as Player.AddItem
    return id + num + BASE_BONUS
end
```

```lua
-- bag.lua
package "Bag"

function UseItem(id) -- exported as Bag.UseItem
    -- zero-require cross-module call to Player package function
    return Player.AddItem(id, 10)
end
```

```lua
-- main.lua
function test()
    local res1 = Player.AddItem(100, 5) -- 106
    local res2 = Bag.UseItem(200)       -- 211
    return res1 + res2                  -- 317
end
```

### Multi-Return & Varargs

- **Multi-return**: Functions can `return a, b` to return multiple values, correctly unpacked in assignments and return statements.
- **Dynamic expansion**: If the last element in a function call or Table constructor is a multi-return function call, its return values are automatically expanded.
- **Varargs (`...`)**: Supports declaring and calling vararg functions. Extra arguments from the C++ side are automatically packed as Multi without manual assembly.
- **C++ return auto-unpacking**: Receive multi-return values via `std::tie(a, b, c)` — templates automatically decompose the Multi CVar into individual variables.

```lua
-- Lua side: define a function supporting varargs and multi-return
function calc_multi(a, ...)
    return a, a * 2, "ok"
end
```

```cpp
// C++ side: pass varargs and unpack multi-return via std::tie
int x = 0, y = 0;
std::string msg;
Call(s, JIT_GCC, "calc_multi", std::tie(x, y, msg), 10, 20, 30); // x=10, y=20, msg="ok"
```

### Built-in Standard Libraries

FakeLua provides a complete core standard library (`math`, `table`, `string`, `os`, `utf8`, `io`, `net`, `event`, `compress`, `crypto`, `csv`, `json`, `mysql`, `sqlite`, `serialize`, `protobuf`), fully designed as independent decoupled C++ modules (`native_math` / `native_table` / `native_string` / `native_os` / `native_utf8` / `native_io` / `native_net` / `native_event` / `native_compress` / `native_crypto` / `native_csv` / `native_json` / `native_mysql` / `native_sqlite` / `native_serialize` / `native_protobuf`), supporting both direct use in Lua scripts and fast-path direct calls from CGen-generated C code:

- **Basic global functions**:
  - **Type & conversion**: `type`, `tostring`, `tonumber`
  - **I/O**: `print`, `select`
  - **Error handling**: `error`, `assert`, `pcall`, `xpcall`
  - **Table iteration**: `next`, `pairs`, `ipairs`
  - **File loading**: `loadfile`, `dofile` (load and compile file, top-level functions registered as globals)
  - **Garbage collection**: `collectgarbage([opt])` (only `"count"` returns KB; other options are no-ops)
- **Version constant**: `_VERSION` (returns `"Fakelua 5.3"`)
- **Math library (`math.*`)**:
  - **Basic & trig**: `math.abs`, `math.floor`, `math.ceil`, `math.min`, `math.max`, `math.sqrt`, `math.sin`, `math.cos`, `math.tan`, `math.asin`, `math.acos`, `math.atan`, `math.sinh`, `math.cosh`, `math.tanh`
  - **Exp/log/decomposition**: `math.exp`, `math.log`, `math.log10`, `math.deg`, `math.rad`, `math.modf`, `math.frexp`, `math.atan2`, `math.copysign`
  - **Random & constants**: `math.random`, `math.randomseed`, plus `math.pi`, `math.huge`, `math.maxinteger`, `math.mininteger`
- **Table library (`table.*`)**:
  - **Array operations**: `table.insert(list [, pos], value)`, `table.remove(list [, pos])`, `table.concat(list [, sep [, i [, j]]])`, `table.sort(list [, comp])`
  - **Pack & unpack**: `table.pack(...)`, `table.unpack(list [, i [, j]])`
  - **Pre-allocated construction**: `table.create(seq_size [, hash_size])`
- **String library (`string.*`)**:
  - **Basic operations**: `string.len`, `string.sub`, `string.rep`, `string.reverse`, `string.lower`, `string.upper`
  - **Encoding conversion**: `string.byte`, `string.char`, `string.charpattern`
  - **Formatting**: `string.format` (supports `%s` `%d` `%i` `%u` `%x` `%X` `%o` `%f` `%e` `%E` `%g` `%G` `%c` `%q` `%p`)
  - **Regex matching**: `string.find`, `string.match`, `string.gmatch`, `string.gsub` (⚠️ **Uses ECMAScript regex syntax, not Lua patterns** — see [Regex Matching](#regex-matching-uses-ecmascript-syntax-not-lua-patterns))
  - **Serialization & loading**: `string.pack`, `string.packsize`, `string.unpack`, `string.dump`, `load`, `loadstring`, `loadfile` (directly compile file, top-level functions registered as globals)
- **OS library (`os.*`)**:
  - **Date & time**: `os.clock()`, `os.date([format[, time]]))` (supports `"*t"` returning table `{year=, month=, day=, hour=, min=, sec=, wday=, yday=, isdst=}`), `os.difftime(t2, t1)`, `os.time([table])`
  - **Environment execution**: `os.execute([command])` (returns `(bool|nil, "exit"|"signal"|"error", code)` triple), `os.exit([code[, close]])`, `os.getenv(varname)`
  - **File operations**: `os.remove(filename)`, `os.rename(oldname, newname)`, `os.tmpname()`
  - **Locale**: `os.setlocale(locale[, category])`
- **UTF-8 library (`utf8.*`)**:
  - **Encoding/decoding**: `utf8.char(...)`, `utf8.codepoint(s [, i [, j]])`, `utf8.codes(s)`
  - **Length & offset**: `utf8.len(s [, i [, j]])`, `utf8.offset(s, n [, i])`
  - **Pattern constant**: `utf8.charpattern`
- **IO library (`io.*`)**:
  - **File open/close**: `io.open(filename [, mode])`, `io.close([file])`, `io.tmpfile()`, `io.popen(command [, mode])` (pipe to external command)
  - **Read/write**: `io.read([format ...])` (supports multi-format args, multi-return), `io.write(...)`, `io.flush()`
  - **File seeking**: `file:seek([whence [, offset]])`, `file:setvbuf(mode [, size])` (returns file on success, nil+errmsg on failure)
  - **Type check**: `io.type(v)`
  - **Standard streams**: `io.stdin`, `io.stdout`, `io.stderr`
  - **File methods**: `file:read([format])`, `file:write(...)`, `file:flush()`, `file:close()`, `file:seek(...)`, `file:setvbuf(...)`, `file:lines()` (line iterator for `for line in file:lines() do ... end`)
- **Net library (`net.*`)**:
  - **Server & client creation**: `net.server(config)`, `net.client(config)` (supports `port`, `maxconn`, `backlog`, `nonblocking`, `nodelay`, `keepalive`)
  - **Framing protocols**: Seamlessly switch via `framer` config:
    - `"header4"` / `"header4_be"` (default): 4-byte big-endian length header
    - `"header4_le"`: 4-byte little-endian length header
    - `"header2"` / `"header2_be"`: 2-byte big-endian length header
    - `"header2_le"`: 2-byte little-endian length header
    - `"line"`: newline (`\n` or `\r\n`) delimited, auto-stripped
    - `"fixed"`: fixed-length protocol (with `fixed_len = N`)
    - `"raw"`: raw passthrough mode
  - **Custom parser**: 
    - **Lua custom parser**: Pass `parser = "Package.my_parser"`, receives buffer string, returns `(payload, consumed_bytes)` or `nil` (insufficient data)
    - **C++ custom parser**: Set `custom_parser_fn` and `custom_encoder_fn` in C++ `NetConfig`
  - **Event dispatch & driver**: `server:dispatch("Package.on_event")` / `client:dispatch(...)` (register unified event callback entry), `server:tick()` / `client:tick()` (drive non-blocking I/O and event dispatch)
  - **Data send & connection management**: `server:send(connid, data)`, `client:send(data)`, `server:close()`, `client:close()`
  - **Status & statistics**: `obj:get_conn_count()`, `obj:get_recv_count()`, `obj:get_last_data()`, `server:get_connid()`, `obj:get_events()`
- **Timer library (`timer.*`)**:
  - **One-shot timer**: `timer.set(delay_ms, "Package.callback")` (register timer, returns `timer_id`; dispatches by function name on expiry; callback signature `function cb(type, timer_id)` where `type == "timer"`), `timer.del(timer_id)` (delete pending timer)
  - **Driver**: `timer.tick()` (call in main loop to fire due timers and heartbeats)
  - **Periodic heartbeat**: `timer.set_heartbeat(interval_ms, "Package.heartbeat_cb")` (global heartbeat, auto-reschedules on expiry; repeated calls overwrite previous heartbeat)
  - **Shared state**: Tests can create global NativeObjects via `new_global_obj(key, type)` / `get_global_obj(key)` (indexed by string key, similar to `_G.xxx`), register `get_int`/`set_int`/`add_int` methods via `timer.register_obj_methods(obj)` for callbacks to record state and tests to verify in main (fakelua has no mutable globals; NativeObject replaces `_G`); `add_int(key, delta)` increments field by delta, starting from 0 if the field doesn't exist
- **Event library (`event.*`)**:
  - **Subscription management**: `event.on(event_name, "Package.callback")` (subscribe), `event.once(event_name, "Package.callback")` (subscribe once, auto-remove after fire), `event.off(event_name, "Package.callback")` (unsubscribe)
  - **Event dispatch**: `event.emit(event_name, arg1, arg2, arg3, arg4)` (fire event, calls all subscribers in order; vararg, up to 4 args forwarded)
  - **Cleanup**: `event.clear(event_name)` (remove all handlers for an event), `event.clear_all()` (remove all handlers for all events)
  - **Re-entrancy safe**: `emit` snapshots the handler list before iteration, so handlers can safely call `on`/`off`/`emit` during dispatch
  - **Typical use**: Game logic layer decoupling (unit death events, building completion events, turn-switch events, etc.)
- **Compress library (`compress.*`)**:
  - **LZ4**: `compress.lz4_compress(data)` (LZ4 frame compression, output embeds original size), `compress.lz4_decompress(data)`
  - **zlib**: `compress.zlib_compress(data, level?)` (zlib deflate, level 1-9, default 6), `compress.zlib_decompress(data)`
  - **gzip**: `compress.gzip_compress(data, level?)` (gzip format, level 1-9, default 6), `compress.gzip_decompress(data)`
  - **Zstd**: `compress.zstd_compress(data, level?)` (Zstandard, level 1-22, default 3), `compress.zstd_decompress(data)`
- **Crypto library (`crypto.*`)**:
  - **Hash functions**: `crypto.md5(data)` (hex-encoded 128-bit digest), `crypto.sha1(data)` (hex-encoded 160-bit), `crypto.sha256(data)` (hex-encoded 256-bit)
  - **Encoding**: `crypto.hex_encode(data)` (binary→hex), `crypto.hex_decode(hex)` (hex→binary), `crypto.base64_encode(data)` (binary→base64, RFC 4648), `crypto.base64_decode(data)` (base64→binary)
  - **AES symmetric encryption**: `crypto.aes_encrypt_ecb(data, key)` / `crypto.aes_decrypt_ecb` (ECB mode, data must be 16-byte aligned), `crypto.aes_encrypt_cbc(data, key, iv)` / `crypto.aes_decrypt_cbc` (CBC mode, PKCS#7 padding), `crypto.aes_encrypt_ctr(data, key, iv)` / `crypto.aes_decrypt_ctr` (CTR stream mode, no padding)
  - **Stream/block ciphers**: `crypto.rc4(key, data)` (RC4 stream cipher, encrypt = decrypt), `crypto.blowfish_encrypt(key, data)` / `crypto.blowfish_decrypt` (Blowfish ECB), `crypto.des_encrypt(key, data)` / `crypto.des_decrypt` (DES ECB), `crypto.triple_des_encrypt(key, data)` / `crypto.triple_des_decrypt` (3DES ECB)
- **CSV library (`csv.*`)**:
  - **Decode**: `csv.decode(str, sep?)` (parse CSV string into table of rows; each row is a 1-indexed table of fields; auto-converts fields to numbers; default separator `,`; handles quoted fields, escaped quotes `""`, BOM, `\r\n` line endings)
  - **Encode**: `csv.encode(rows, sep?)` (encode table of rows into CSV string; auto-quotes fields containing `"`, separator, `\n`, or `\r`; default separator `,`)
- **JSON library (`json.*`)**:
  - **Encode**: `json.encode(value)` (Lua value → JSON string; tables with consecutive integer keys 1..N become JSON arrays, others become JSON objects; floats use `%.17g` precision)
  - **Decode**: `json.decode(str)` (JSON string → Lua value; `null` → `nil`, integers/floats/arrays/objects correctly converted)
- **MySQL library (`mysql.*`)**:
  - **Direct connection**: `mysql.connect(config, "Package.on_connect")` (config `{host, port, user, password, db}`, async connect), `conn:query(sql, "Package.on_result")` (async query, result is table of rows), `conn:stmt_prepare/sql/stmt_execute/stmt_close` (prepared statements), `conn:tick()` (pump network events), `conn:close()`
  - **Connection pool**: `mysql_pool.create(config)` (config additionally supports `pool_size`, `timeout_ms`, `heartbeat_ms`, `max_retries`), `pool:acquire()/release()/tick()/close()/stats()`
  - All operations are async callback-based; callback signature: `function cb(err, result)`
- **SQLite library (`sqlite.*`)**:
  - **Open database**: `sqlite.open(filename)` (open or create SQLite database, returns db object)
  - **Execute SQL**: `db:exec(sql)` (execute SQL; SELECT returns table of rows, non-SELECT returns nil)
  - **Prepared statements**: `db:prepare(sql)` (returns stmt object), `stmt:bind(...)` (bind params, supports nil/int/float/bool/string), `stmt:step()` (execute and return next row, returns nil when done for SELECT), `stmt:reset()` (reset statement preserving bindings), `stmt:columns()` (return column name table), `stmt:close()`
  - **Helper methods**: `db:last_insert_rowid()` (last insert rowid), `db:changes()` (rows affected by last statement), `db:close()`
  - All operations synchronous, based on SQLite3 amalgamation source
- **Object system (Lua-side API)**:
  - **Group management**: `new_native_group()` (create group, returns `group_id`), `del_native_group(group_id)` (batch-destroy all objects in group, returns count)
  - **Object creation & lookup**: `new_native_obj(group_id, type, id)` (create object in group), `get_native_obj(type, id)` (find by type+id), `new_global_obj(key, type)` (create global object), `get_global_obj(key)` (find global object), `del_global_obj(key)` (destroy global object)
- **Serialize library (`serialize.*`)**:
  - **Binary serialize/deserialize**: `serialize.encode(value)` encodes Lua values to binary strings; `serialize.decode(data)` deserializes back — they are inverse operations
  - **Protobuf-like encoding**: Integers use zigzag + varint encoding (small integers use fewer bytes); floats use little-endian 8-byte memcpy; strings use deduplication (identical strings store a varint reference ID from the second occurrence); tables recursively serialize as key-value sequences
  - **Supported types**: `nil`, `boolean`, integer, float, string (binary-safe), table (nested); unsupported types like functions (closures) in tables are skipped; passing an unsupported type at top level errors
  - **Typical use**: Zero-copy conversion between network messages and Lua tables, game state persistence snapshots
- **Protobuf library (`protobuf.*`)**:
  - **Runtime .proto parsing**: `protobuf.load(proto_text)` dynamically parses proto3 text, registers all message/enum definitions to global schema registry
  - **Standard protobuf encode/decode**: `protobuf.encode(message_name, table)` packs a Lua table into standard protobuf binary per proto definition; `protobuf.decode(message_name, binary)` deserializes back — fully interoperable with official protobuf
  - **Schema query**: `protobuf.types()` returns registered message names; `protobuf.fields(message_name)` returns field info (name/number/type/label)
  - **Supported proto3 features**: message (nested), enum, map\<K,V\>, oneof, repeated (packed by default), optional (explicit presence), all 18 scalar types, import (multi-file)
  - **Protobuf-like encoding**: tag = field_number << 3 | wire_type; varint for integers (zigzag for sint); little-endian memcpy for floats; length-prefixed string/bytes/message; packed repeated scalars by default
  - **Typical use**: Cross-language communication in game servers (battle/login/world servers), client protocol interop

```lua
-- Example: sorting, formatting, and math using standard libraries
local scores = { 85, 92, 78, 95 }
table.sort(scores, function(a, b) return a > b end) -- descending sort

local top_student = string.format("Top score: %d, Angle Rad: %.2f", scores[1], math.rad(180))
local info = table.concat(scores, ", ")
-- top_student => "Top score: 95, Angle Rad: 3.14"
-- info        => "95, 92, 85, 78"
```

#### Regex Matching: ECMAScript Syntax, Not Lua Patterns

`string.find` / `string.match` / `string.gmatch` / `string.gsub` are implemented by `std::regex` (`std::regex::ECMAScript`) under the hood, **not** Lua's native pattern engine. When migrating scripts from standard Lua, patterns must be rewritten:

| Purpose | Lua Pattern | FakeLua (ECMAScript Regex) |
|---|---|---|
| Digits | `%d` | `\\d` |
| Letters | `%a` | `[A-Za-z]` |
| Alphanumeric | `%w` | `[A-Za-z0-9]` (note `\\w` additionally includes `_`) |
| Whitespace | `%s` | `\\s` |
| Escape literal | `%.`, `%%` | `\\.`、`%` |
| Lazy repeat | `-` (e.g. `.-`) | `?` (e.g. `.*?`) |
| Backreference in replacement | `%1`, `%0` | `$1`, `$&` |

> Since `\d` is not a valid escape in Lua string literals, backslashes in regex patterns must be written as `"\\d+"`. FakeLua does not support `[[...]]` long strings as a workaround.
>
> For scripts that need to be compatible with both standard Lua and FakeLua, use syntax that has the same semantics in both engines, e.g. `[0-9]+` instead of `%d+`, `[A-Za-z]+` instead of `%a+` — bracket character classes, `+`, `*`, `()` capture groups have consistent meaning in both.

Key differences:

- **`gsub` replacement strings** use JS-style notation: `$1`…`$9` (capture groups), `$&` (entire match), `` $` `` (text before match), `$'` (text after match), `$$` (literal `$`). Lua's `%1` / `%0` are treated as literal characters here.
- **Lua-pattern-only features unavailable**: Alternation `|`, non-greedy quantifiers `*?` `+?`, range repeats `{n,m}`, lookahead `(?=...)` and other ECMAScript features are available out of the box.
- **Lua-specific syntax unsupported**: `%b()` (balanced match), `%f[set]` (frontier pattern), and all `%` character classes.
- **Invalid patterns don't throw**: `std::regex_error` is caught and returns `nil`, so the script doesn't interrupt — incorrect patterns silently fail to match rather than erroring.
- **`string.find`'s `plain` parameter** has the same semantics as Lua: passing `true` degrades to pure substring search, completely bypassing the regex engine — also the fastest path.
- **Performance**: The regex path is significantly slower than Lua's native pattern engine (see [benchmark/README.md](benchmark/README.md)); prefer `plain` search or `string.sub` / `string.byte` basic operations on hot paths.

```lua
-- Lua pattern (won't match in FakeLua, returns nil)
local n1 = string.match("abc123", "%d+")      -- nil

-- FakeLua ECMAScript regex
local n2 = string.match("abc123", "\\d+")     -- "123"

-- gsub capture references use $1 instead of %1
local s = string.gsub("hello world", "([a-z]+) ([a-z]+)", "$2 $1")  -- "world hello"
```

### C++ Embedding API

- `CompileFile` / `CompileString` / `Call`, RAII-style `FakeluaStateGuard`
- Supports advanced mapping of basic types, objects, and custom VarInterface implementations
- Supports recording compiled C code for debugging and performance analysis (`CompileConfig::record_c_code`)

```cpp
FakeluaStateGuard guard;
State* s = guard.GetState();
CompileFile(s, "script.lua", CompileConfig{.debug_mode = false});

int sum = 0;
Call(s, JIT_GCC, "add", sum, 10, 20); // embed-call a Lua function
```

### Closures & Upvalue Capture

FakeLua fully supports Lua closures and upvalue capture:

- **Static upvalue analysis**: [ResolveScopes](src/compile/c_gen.cpp) static AST pass automatically derives scope and cross-function capture relationships for all local variables, parameters, and loop variables.
- **Heap boxing sharing**: Captured variables are automatically promoted to heap-allocated `CVar *` boxes at definition time; multiple closures share the same heap pointer, naturally synchronizing shared upvalues within the same scope.
- **Anonymous functions & higher-order functions**: Supports anonymous function expressions `function(args) body end` as values (higher-order functions like `map`), and arbitrary callee calls (e.g., `tbl[key]()` or `(fn)()` chains).
- **Colon method calls**: Full support for `obj:method(args)` colon syntax sugar — the evaluator implicitly passes the caller as the first `self` parameter to the target closure in JIT codegen.
- **Generic `for in` iterator**: Full support for Lua generic `for var1, ..., varn in explist do` iterators (preserving native C struct-optimized loops for `pairs`/`ipairs` while fully supporting stateless iterators and closure generators).
- **Per-iteration loop variable capture**: In `for` and `for in` loops, loop variables are automatically re-boxed each iteration, ensuring closures created inside the loop bind independent variable copies.

```lua
-- Higher-order function and closure counter example
function make_counter(start)
    local count = start
    return function()
        count = count + 1
        return count
    end
end

local counter = make_counter(10)
print(counter()) -- 11
print(counter()) -- 12
```

### Complex Global Variable Initialization

Supports arbitrary complex expressions as global/file-level variable initializers:

```lua
local x = math.floor(3.14) + 1
local y = x * 2 - 1
local z = (x + y) / 2.0
```

The compiler extracts complex initializers into a generated `__fakelua_init()` function, executed immediately after JIT loading to give globals their correct runtime values.

### File-Level Statement Restrictions

File-level (chunk top) only allows three types of statements: an optional first-line `package "Name"` declaration, `local` variable definitions, and function definitions.
`if` / `while` / `for` / assignment and other executable statements must be inside function bodies, otherwise [semantic_analysis](src/compile/semantic_analysis.h)'s
`CheckFileLevelStmts` stage (before AST rewrite) errors directly:

```lua
local x = 5
if x > 3 then -- compile error: unsupported file-level statement If
    x = 10
end
```

File-level `local` is treated as a const for that file, downgraded to C `static const`, and therefore cannot be reassigned at file level.

## Known Limitations

### Type System Limitations
- Type inference is based on static analysis; complex dynamic type operations cannot be optimized
- Function specialization is based on math parameter discovery at call sites
- Function parameter limit: 32 (configured via `kMaxFunctionInputParams`)
- Math specialization parameter limit: 8 (configured via [kMaxMathSpecializedParams](include/fakelua.h); math params beyond this limit are not specialized and treated as generic dynamic params)

### Missing Language Features
- No coroutine support
- No metatable support
- No `require` / `module` system (note: fakelua has its own `package "Name"` modular mechanism)
- No `rawequal` / `rawget` / `rawset` / `rawlen` (meaningless without metatables)
- No debug standard library

### Standard Library Semantic Differences
- `string.find` / `match` / `gmatch` / `gsub` use **ECMAScript regex** not Lua patterns: `%d`, `%b()`, `%f[]` and other Lua-specific syntax are unavailable; `gsub` replacement strings must use `$1` instead of `%1` (see [Regex Matching](#regex-matching-uses-ecmascript-syntax-not-lua-patterns))
- No implicit string→number conversion in arithmetic: `"10" + 1` works in Lua but errors in FakeLua

## Quick Start

### Building

#### System Requirements
- **C++23** compiler (GCC 11+ / Clang 16+ / MSVC 2022+)
- CMake 3.5+
- make or ninja

#### Linux / macOS

```bash
cmake -S . -B build
cmake --build build --parallel
```

> On macOS, first `brew install lua cmake` and add `-DCMAKE_PREFIX_PATH="$(brew --prefix)"` to the cmake command.

Build only core library and CLI tools (no tests/benchmarks):

```bash
cmake --build build --target fakelua flua --parallel
```

#### Windows (MSYS2 + MinGW)

```bash
cmake -S . -B build -G Ninja
cmake --build build --parallel
ctest --test-dir build -V
```

### Testing & Benchmarks

```bash
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build --parallel
ctest --test-dir build -V
./build/bin/bench_mark
```

> Unit tests and benchmarks require the Lua development package (header `lua.h` and library files).
> - Linux: `sudo apt-get install liblua5.4-dev` or `liblua5.3-dev`
> - macOS: `brew install lua`
> - Windows MSYS2: `pacman -S mingw-w64-x86_64-lua`

### CLI Tool `flua`

```bash
./build/bin/flua <script.lua> --entry=<func> --jit_type=<0|1> --repeat=<N>
```

- `--entry`: Entry function name (default `main`)
- `--jit_type`: `0`=TCC, `1`=GCC
- `--repeat`: Repeat call count (for performance measurement)
- `--debug`: Enable debug mode (default `false`; when `true`, outputs generated C source)

## Performance Benchmarks

Comparing Lua 5.4, FakeLua TCC, FakeLua GCC across 11 algorithms: Fibonacci, GCD, fast power, linear sum, bubble sort, prime sieve, etc. (Release `-O3` mode):

| Algorithm (typical params) | Lua 5.4 | FakeLua TCC | FakeLua GCC |
|---|---|---|---|
| Fibonacci n=32 | 297.9 ms | 26.7 ms (**11.2x**↑) | 6.8 ms (**43.5x**↑) |
| Sum n=5000000 | 33.9 ms | 18.4 ms (**1.8x**↑) | 2.0 ms (**17.1x**↑) |
| Popcount n=100000 | 18.2 ms | 3.1 ms (**5.9x**↑) | 974.0 μs (**18.7x**↑) |
| BubbleSort n=200 | 1.5 ms | 3.3 ms (0.45x) | 738.8 μs (**2.0x**↑) |
| Sieve n=5000 | 353.4 μs | 1.0 ms (0.34x) | 219.3 μs (**1.6x**↑) |

> TCC is generally faster than Lua for pure computation; in Table-operation-heavy scenarios, Table struct specialization gives both GCC and TCC a significant boost in Table read/write performance. Table standard library (`table.insert`/`remove`/`sort`) on the GCC backend is also 3.6–4.2x faster than Lua. Full data available in [benchmark/README.md](benchmark/README.md).

## C++ API Reference

### State Management

```cpp
// Manual management (not recommended — easy to leak)
State* s = FakeluaNewState(StateConfig{});
// ... use s ...
FakeluaDeleteState(s);

// Or RAII style (recommended)
FakeluaStateGuard guard(StateConfig{});
State* s = guard.GetState();
// ... use s ...
// automatically freed
```

### API Overview

| Function | Description |
|------|------|
| `FakeluaNewState()` | Create FakeLua state |
| `FakeluaDeleteState()` | Free FakeLua state |
| `CompileFile()` | Compile a Lua file |
| `CompileString()` | Compile a Lua code string |
| `Call()` | Invoke a compiled function |
| `GetLastRecordedCCode()` | Get the most recently compiled C code |
| `SetVarInterfaceNewFunc()` | Set custom VarInterface factory |
| `SetDebugLogLevel()` | Set global debug log level |

### Type Conversion

FakeLua provides `inter::NativeToFakelua()` and `FakeluaToNative()` for automatic deduced conversion:

```cpp
// Native → FakeLua
CVar v_int = inter::NativeToFakelua(s, 42);
CVar v_str = inter::NativeToFakelua(s, std::string("hello"));
CVar v_bool = inter::NativeToFakelua(s, true);

// FakeLua → Native
int native_int = inter::FakeluaToNative<int>(v_int);
std::string native_str = inter::FakeluaToNative<std::string>(v_str);
```

### Table ↔ Object Mapping

Implementing [VarInterface](include/fakelua.h) enables bidirectional mapping between Lua tables and native objects:

```cpp
class CustomVar : public VarInterface {
    // implement all virtual functions...
};

// Register factory function
SetVarInterfaceNewFunc(s, []() { return new CustomVar(); });

// Table-type arguments passed in Call will automatically construct CustomVar instances
```

## Architecture Overview

### Compilation Pipeline

```
Lua source
   ↓
[Lexing] → tokens (flexer)
   ↓
[Parsing] → AST (bison + syntax_tree)
   ↓
[File-level stmt check] → reject non-declaration statements (semantic_analysis)
   ↓
[Preprocessing] → normalized AST (preprocessor)
   ↓
[Semantic analysis] → analysis result (semantic_analysis)
   ↓
[Type inference] → type hints (type_inferencer)
   ↓
[C code generation] → C source (c_gen)
   ↓
[JIT compilation] → machine code (tcc_jit / gcc_jit)
   ↓
[Load & execute] → result
```

### Key Components

| Module | Responsibility |
|------|------|
| `lexer/parser` | Lua lexing and parsing |
| `syntax_tree` | AST representation and traversal |
| `preprocessor` | Lua syntax normalization (e.g., functiondef hoisting) |
| `semantic_analysis` | Semantic and control flow analysis (undefined symbol analysis, etc.) |
| `type_inferencer` | Static type inference and specialization decisions |
| `c_gen` | C code generation and type-driven optimization |
| `compile_common` | Common type inference and codegen utilities |
| `jit/*` | TCC and GCC backend integration |
| `state` | FakeLua runtime state management |
| `var` | Dynamic value CVar and conversion utilities |

## FAQ

### Q: Why choose a Lua subset over full Lua?
A: Certain dynamic features of full Lua (e.g., metatables) are difficult to compile efficiently. The subset focuses on statically analyzable common patterns, achieving near-C performance through type inference and JIT compilation. Multi-return, parameter expansion, and varargs are already supported, while metacoroutines and other complex features remain unavailable.

### Q: How to choose between TCC and GCC backends?
A: **GCC** is the primary backend for production (with `-O3` optimization generating high-quality native code); **TCC** compiles extremely fast but with limited optimization, primarily for development, debugging, and testing.

### Q: Can it be used in embedded or constrained environments?
A: Yes — the TCC backend is small and fast, suitable for embedded use. The core library has minimal dependencies (C++ standard library only) and is cross-compilable.

### Q: How to debug generated C code?
A: Enable `CompileConfig::debug_mode` to inspect logs and C code; use `GetLastRecordedCCode()` to export C code for analysis.

### Q: Is multithreading supported?
A: Each `State` is currently thread-local; in multithreaded environments, create an independent `State` per thread.
