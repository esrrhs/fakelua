# FakeLua 内置扩展库

所有内置原生库的详细 API 参考。每个模块位于 `src/native/` 下的独立子目录，包含 `.h` 和 `.cpp` 文件对。

> 高层概览请参阅[主 README](../README.md)。

## 模块一览

| 模块 | 目录 | 说明 |
|------|------|------|
| basic | `basic/` | 全局函数：`print`、`type`、`tostring`、`tonumber`、`select`、`error`、`assert`、`pcall`、`xpcall`、`next`、`pairs`、`ipairs`、`collectgarbage` |
| math | `math/` | 数学函数：算术、三角、指数/对数、随机数、常量 |
| table | `table/` | 表操作：`insert`、`remove`、`concat`、`sort`、`pack`、`unpack`、`move`、`create` |
| string | `string/` | 字符串操作：子串、大小写、模式匹配（ECMAScript 正则）、格式化、二进制 pack/unpack、序列化 |
| os | `os/` | 系统接口：时间、日期、环境变量、文件操作、进程执行 |
| utf8 | `utf8/` | UTF-8 编解码：`char`、`codepoint`、`codes`、`len`、`offset` |
| io | `io/` | 文件 IO：open、close、read、write、seek、popen、标准流 |
| net | `net/` | TCP 网络：服务端/客户端、帧协议、自定义解析器、异步事件分发 |
| timer | `timer/` | 定时器：一次性、周期心跳，由 `tick()` 驱动 |
| event | `event/` | 发布/订阅事件系统：`on`、`once`、`off`、`emit`、`clear`、`clear_all` |
| random | `random/` | 可种子随机数（PCG-32）：`int`、`float`、`dice`、`chance`、`weighted`、`get_state`、`set_state` |
| compress | `compress/` | 压缩：LZ4、zlib、gzip、Zstd |
| crypto | `crypto/` | 加解密：MD5/SHA1/SHA256、hex/base64、AES/RC4/Blowfish/DES/3DES |
| csv | `csv/` | CSV 解码/编码 |
| json | `json/` | JSON 编码/解码 |
| yaml | `yaml/` | YAML 解码/编码（yaml-cpp） |
| toml | `toml/` | TOML 解码/编码（toml++） |
| xml | `xml/` | XML 解码/编码（pugixml） |
| ini | `ini/` | INI 解码/编码（inih） |
| mysql | `mysql/` | 异步 MySQL 客户端：直连 + 连接池 |
| sqlite | `sqlite/` | SQLite3 封装：exec、预处理语句、同步 |
| serialize | `serialize/` | 二进制序列化：zigzag + varint 编码 + 字符串去重 |
| protobuf | `protobuf/` | 运行时 .proto 解析、标准 protobuf3 wire 编码/解码 |
| object | `object/` | NativeObject Lua 侧 API：组管理、对象创建/查找 |

---

## Basic（全局函数）

**文件：** `basic/native_basic.h` · **注册：** `RegisterBasicLibraryApi`

| 函数 | 参数 | 说明 |
|------|------|------|
| `print(...)` | vararg | 打印所有参数（tab 分隔）到 stdout，带换行 |
| `type(v)` | 1 | 返回类型名：`"nil"`、`"boolean"`、`"number"`、`"string"`、`"table"`、`"function"`、`"userdata"` |
| `tostring(v)` | 1 | 值转字符串 |
| `tonumber(v [, base])` | 1-2 | 字符串转数字，可选进制（2-36） |
| `select(n, ...)` | vararg | 选取第 `n` 个起的参数；`select("#", ...)` 返回参数个数 |
| `error(msg [, level])` | 1-2 | 抛出错误 |
| `assert(v, ...)` | vararg | `v` 为假则抛错；否则返回所有参数 |
| `pcall(f, ...)` | vararg | 安全调用；返回 `true, result...` 或 `false, errmsg` |
| `xpcall(f, msgh, ...)` | vararg | 带错误处理器的安全调用 |
| `next(t, ...)` | vararg | 遍历表的下一个键值对 |
| `pairs(t)` | 1 | 通用遍历迭代器 |
| `ipairs(t)` | 1 | 整数索引遍历迭代器 |
| `collectgarbage([opt])` | vararg | 仅 `"count"` 返回内存 KB；其他选项为空操作 |

---

## Math（数学）

**文件：** `math/native_math.h` · **注册：** `RegisterMathLibraryApi`

| 函数 | 参数 | 说明 |
|------|------|------|
| `math.abs(x)` | 1 | 绝对值（处理 INT64_MIN） |
| `math.floor(x)` | 1 | 向下取整 |
| `math.ceil(x)` | 1 | 向上取整 |
| `math.max(...)` | vararg | 最大值 |
| `math.min(...)` | vararg | 最小值 |
| `math.sqrt(x)` | 1 | 平方根 |
| `math.sin/cos/tan(x)` | 1 | 三角函数 |
| `math.asin/acos/atan(x)` | 1 | 反三角函数 |
| `math.atan2(y, x)` | 2 | 双参数反正切 |
| `math.sinh/cosh/tanh(x)` | 1 | 双曲函数 |
| `math.exp(x)` | 1 | 指数 e^x |
| `math.log(x [, base])` | 1-2 | 对数，可选底数 |
| `math.log10(x)` | 1 | 常用对数 |
| `math.pow(x, y)` | 2 | 幂运算 x^y |
| `math.fmod(x, y)` | 2 | 浮点取模 |
| `math.ldexp(x, exp)` | 2 | x * 2^exp |
| `math.modf(x)` | 1 | 整数和小数部分 |
| `math.frexp(x)` | 1 | 尾数和指数 |
| `math.deg(x)` | 1 | 弧度转角度 |
| `math.rad(x)` | 1 | 角度转弧度 |
| `math.copysign(x, y)` | 2 | 复制符号 |
| `math.type(x)` | 1 | 返回 `"integer"`、`"float"` 或 nil |
| `math.tointeger(x)` | 1 | 无损转整数 |
| `math.ult(x, y)` | 2 | 无符号小于比较 |
| `math.random(...)` | vararg | 随机数：0 参 [0,1)，1 参 [1,u]，2 参 [l,u] |
| `math.randomseed(...)` | vararg | 设置随机种子 |

**常量：** `math.pi`、`math.huge`、`math.maxinteger`、`math.mininteger`

---

## Table（表操作）

**文件：** `table/native_table.h` · **注册：** `RegisterTableLibraryApi`

| 函数 | 参数 | 说明 |
|------|------|------|
| `table.insert(t, [pos,] val)` | vararg | 在指定位置或末尾插入值 |
| `table.remove(t, [pos])` | vararg | 删除指定位置或末尾元素，返回该元素 |
| `table.concat(t, [sep, [i, [j]]])` | vararg | 拼接元素，可选分隔符和范围 |
| `table.unpack(t, [i, [j]])` | vararg | 解包范围内的元素 |
| `table.pack(...)` | vararg | 将参数打包为带 `n` 字段的表 |
| `table.move(t1, f, e, t, [t2])` | vararg | 在表之间移动元素 |
| `table.sort(t, [comp])` | vararg | 原地排序，可选比较器 |
| `table.create(n, [val])` | vararg | 创建预分配大小的表，可选填充值 |

---

## String（字符串）

**文件：** `string/native_string.h` · **注册：** `RegisterStringLibraryApi`

| 函数 | 参数 | 说明 |
|------|------|------|
| `string.len(s)` | 1 | 字节长度 |
| `string.sub(s, i, [j])` | 2-3 | 子串，1-based 索引，支持负数 |
| `string.rep(s, n, [sep])` | 2-3 | 重复字符串 n 次，可选分隔符 |
| `string.reverse(s)` | 1 | 反转字符串 |
| `string.lower(s)` | 1 | ASCII 小写 |
| `string.upper(s)` | 1 | ASCII 大写 |
| `string.byte(s, [i, [j]])` | vararg | 范围内的字节值 |
| `string.char(...)` | vararg | 编码点 0-255 转字符 |
| `string.format(fmt, ...)` | vararg | 格式化输出（支持 `%s %d %i %u %x %X %o %f %e %E %g %G %c %q %p`） |
| `string.find(s, pattern, [init, [plain]])` | vararg | 正则或纯子串查找；返回位置 + 捕获 |
| `string.match(s, pattern, [init])` | vararg | 正则匹配；返回捕获或完整匹配 |
| `string.gmatch(s, pattern)` | 2 | 正则匹配迭代器 |
| `string.gsub(s, pattern, repl, [n])` | vararg | 正则替换；支持字符串/函数/表替换 |
| `string.dump(f, [strip])` | vararg | 将闭包序列化为二进制字符串 |
| `load(source, ...)` | vararg | 编译 Lua 源码为闭包 |
| `loadstring(s, ...)` | vararg | `load` 别名 |
| `loadfile(file, ...)` | vararg | 加载并编译 Lua 文件 |
| `string.pack(fmt, ...)` | vararg | 二进制打包（Lua 5.3 格式） |
| `string.packsize(fmt)` | 1 | 计算格式打包后大小 |
| `string.unpack(fmt, s, [pos])` | vararg | 二进制解包（Lua 5.3 格式） |

> ⚠️ `string.find`/`match`/`gmatch`/`gsub` 底层使用 **ECMAScript 正则**（`std::regex::ECMAScript`），而非 Lua pattern。参见主 README 的[正则匹配](../README.md#regex-matching-uses-ecmascript-syntax-not-lua-patterns)章节。

---

## OS（系统接口）

**文件：** `os/native_os.h` · **注册：** `RegisterOsLibraryApi`

| 函数 | 参数 | 说明 |
|------|------|------|
| `os.clock()` | 0 | CPU 时间（秒） |
| `os.date([fmt, [time]])` | vararg | 格式化日期/时间；`"*t"` 返回表 `{year, month, day, hour, min, sec, wday, yday, isdst}` |
| `os.difftime(t2, t1)` | 2 | 两个时间戳之差 |
| `os.execute([cmd])` | vararg | 执行 shell 命令；返回 `(status_bool_or_nil, "exit"|"signal"|"error", code)` 三元组 |
| `os.exit([code, [close]])` | vararg | 终止进程 |
| `os.getenv(name)` | 1 | 获取环境变量 |
| `os.remove(filename)` | 1 | 删除文件 |
| `os.rename(old, new)` | 2 | 重命名文件 |
| `os.setlocale(locale, [cat])` | vararg | 设置/查询区域 |
| `os.time([table])` | vararg | 当前时间或从表构建时间戳 |
| `os.tmpname()` | 0 | 生成安全临时文件名 |

---

## UTF-8

**文件：** `utf8/native_utf8.h` · **注册：** `RegisterUtf8LibraryApi`

| 函数 | 参数 | 说明 |
|------|------|------|
| `utf8.char(...)` | vararg | 编码点转 UTF-8 字符串 |
| `utf8.codepoint(s, [i, [j]])` | vararg | 范围内的编码点 |
| `utf8.codes(s)` | 1 | 简化迭代器支持 |
| `utf8.len(s, [i, [j]])` | vararg | 字符数；非法字节返回 nil + 位置 |
| `utf8.offset(s, n, [i])` | vararg | 第 n 个字符的字节位置 |

**常量：** `utf8.charpattern`

---

## IO（文件 IO）

**文件：** `io/native_io.h` · **注册：** `RegisterIoLibraryApi`

| 函数 | 参数 | 说明 |
|------|------|------|
| `io.open(filename, [mode])` | vararg | 打开文件；返回文件对象或 nil, err, errno |
| `io.close([file])` | vararg | 关闭文件（默认刷新 stdout） |
| `io.read(...)` | vararg | 从 stdin 按格式读取 |
| `io.write(...)` | vararg | 写入 stdout |
| `io.flush()` | 0 | 刷新 stdout |
| `io.type(v)` | 1 | 返回 `"file"`、`"closed file"` 或 nil |
| `io.tmpfile()` | 0 | 创建临时文件 |
| `io.popen(cmd, [mode])` | vararg | 打开进程管道 |
| `io.input([file])` | vararg | 设置/获取默认输入文件 |
| `io.output([file])` | vararg | 设置/获取默认输出文件 |
| `io.lines([file, ...])` | vararg | 打开文件返回行迭代器 |
| `io.stdin/stdout/stderr` | 0 | 标准流文件对象 |

**文件对象方法**（类型 `iofile`）：

| 方法 | 说明 |
|------|------|
| `file:read([format...])` | 按格式读取 |
| `file:write(...)` | 写入值，返回 self |
| `file:flush()` | 刷新缓冲区 |
| `file:close()` | 关闭文件 |
| `file:seek([whence, [offset]])` | 定位；返回位置 |
| `file:setvbuf(mode, [size])` | 设置缓冲（`"no"`、`"full"`、`"line"`） |
| `file:lines()` | 行迭代器闭包 |

---

## Net（TCP 网络）

**文件：** `net/native_net.h` · **注册：** `RegisterNetLibraryApi`

**配置表字段：** `ip`、`port`、`maxconn`、`backlog`、`nonblocking`、`nodelay`、`keepalive`、`framer`、`parser`、`fixed_len`、`ws_path`、`ws_host`、`ws_origin`

**帧协议：**

| 协议 | 说明 |
|------|------|
| `header4` / `header4_be` | 4 字节大端长度头（默认） |
| `header4_le` | 4 字节小端长度头 |
| `header2` / `header2_be` | 2 字节大端长度头 |
| `header2_le` | 2 字节小端长度头 |
| `line` | 换行符分隔，自动去除 |
| `fixed` | 固定长度（需 `fixed_len = N`） |
| `raw` | 原始透传 |
| `websocket` / `ws` | RFC 6455 WebSocket（文本帧） |

**自定义解析器：** `parser = "Package.func"`（Lua）或 `custom_parser_fn`/`custom_encoder_fn`（C++ `NetConfig`）

**WebSocket 额外配置：** `ws_path`（默认 `"/"`）、`ws_host`（客户端 Host，默认 `ip:port`）、`ws_origin`（可选）

| 函数/方法 | 说明 |
|------|------|
| `net.server(config)` | 创建 TCP 服务端 |
| `net.client(config)` | 创建 TCP 客户端 |
| `net.ws_server(config)` | 创建 WebSocket 服务端（等价于 `framer="websocket"`） |
| `net.ws_client(config)` | 创建 WebSocket 客户端 |
| `obj:dispatch(func_name)` | 注册 Lua 回调函数名 |
| `obj:tick()` | 驱动 IO 和事件分发 |
| `obj:send(connid, data)` | 发送数据（服务端需指定 connid；客户端省略） |
| `obj:close()` | 关闭连接/服务端 |
| `obj:close_connection(connid)` | 关闭单个连接（仅服务端） |
| `obj:get_events()` | 事件历史 |
| `obj:get_last_data()` | 最近接收数据 |
| `obj:get_conn_count()` | 连接数 |
| `obj:get_recv_count()` | 收包数 |
| `obj:get_connid()` | 最近连接 ID（仅服务端） |

---

## Timer（定时器）

**文件：** `timer/native_timer.h` · **注册：** `RegisterTimerLibraryApi`

| 函数 | 参数 | 说明 |
|------|------|------|
| `timer.set(delay_ms, func_name)` | 2 | 一次性定时器；返回 `timer_id` |
| `timer.del(timer_id)` | 1 | 删除待触发定时器 |
| `timer.tick()` | 0 | 触发到期定时器和心跳 |
| `timer.set_heartbeat(interval_ms, func_name)` | 2 | 周期心跳；自动重调度，覆盖前一个 |
| `timer.register_obj_methods(obj)` | 1 | 在 NativeObject 上注册 `get_int`/`set_int`/`add_int` 共享状态 |

**回调签名：** `function cb(type, timer_id)`，其中 `type == "timer"`

---

## Event（事件系统）

**文件：** `event/native_event.h` · **注册：** `RegisterEventLibraryApi`

| 函数 | 参数 | 说明 |
|------|------|------|
| `event.on(event_name, func_name)` | 2 | 订阅处理器 |
| `event.once(event_name, func_name)` | 2 | 订阅一次性（触发后自动移除） |
| `event.off(event_name, func_name)` | 2 | 取消订阅 |
| `event.emit(event_name, ...)` | vararg | 触发事件；最多 4 个参数转发给处理器 |
| `event.clear(event_name)` | 1 | 移除事件的所有处理器 |
| `event.clear_all()` | 0 | 移除所有事件的所有处理器 |

> 可重入安全：`emit` 在迭代前快照处理器列表。

---

## Random（随机数）

**文件：** `random/native_random.h` · **注册：** `RegisterRandomLibraryApi`

PCG-32 算法：64-bit 状态，32-bit 输出，周期 2^64。每个 `random.new(seed)` 创建独立的 RNG 流，由 NativeObject 支撑。不同种子产生不同序列；相同种子产生相同序列。状态序列化为 hex 字符串，避免 32-bit 截断。

| 函数 | 参数 | 说明 |
|------|------|------|
| `random.new(seed)` | 1 | 用给定 seed（整数）创建 RNG 实例 |
| `rng:int(min, max)` | 2 | [min, max] 均匀整数（含端点） |
| `rng:float(min, max)` | 2 | [min, max) 均匀浮点数 |
| `rng:dice(count, sides)` | 2 | `count` 个骰子的总和，每个 [1, sides] |
| `rng:chance(prob)` | 1 | 以概率 `prob`（0.0–1.0）返回 `true` |
| `rng:weighted(weights)` | 1 | 从权重表选取 1-based 下标；零权重项不会被选中 |
| `rng:get_state()` | 0 | 获取 64-bit 内部状态，返回 hex 字符串（如 `"0x1234567890ABCDEF"`） |
| `rng:set_state(hex_str)` | 1 | 从 hex 字符串恢复 64-bit 内部状态 |

> **多流独立：** 为不同系统创建独立 RNG 实例（如战斗 RNG、掉落 RNG、事件 RNG）。使用单一全局 RNG 会导致跨系统关联——消耗一个系统的随机数会改变所有其他系统的序列。

> **存档/读档：** 使用 `get_state()` / `set_state()` 配合 hex 字符串进行序列化。hex 字符串是 JSON 安全的，且完整保留 64-bit 状态无精度损失。

---

## Compress（压缩）

**文件：** `compress/native_compress.h` · **注册：** `RegisterCompressLibraryApi`

| 函数 | 参数 | 说明 |
|------|------|------|
| `compress.lz4_compress(data)` | 1 | LZ4 frame 压缩（内嵌原始大小） |
| `compress.lz4_decompress(data)` | 1 | LZ4 解压 |
| `compress.zlib_compress(data, [level])` | 1-2 | zlib deflate，级别 1-9，默认 6 |
| `compress.zlib_decompress(data)` | 1 | zlib inflate |
| `compress.gzip_compress(data, [level])` | 1-2 | gzip 压缩，级别 1-9，默认 6 |
| `compress.gzip_decompress(data)` | 1 | gzip 解压 |
| `compress.zstd_compress(data, [level])` | 1-2 | Zstandard，级别 1-22，默认 3 |
| `compress.zstd_decompress(data)` | 1 | Zstandard 解压 |

---

## Crypto（加解密）

**文件：** `crypto/native_crypto.h` · **注册：** `RegisterCryptoLibraryApi`

| 函数 | 参数 | 说明 |
|------|------|------|
| `crypto.md5(data)` | 1 | MD5 哈希 → hex 字符串 |
| `crypto.sha1(data)` | 1 | SHA-1 哈希 → hex 字符串 |
| `crypto.sha256(data)` | 1 | SHA-256 哈希 → hex 字符串 |
| `crypto.hex_encode(data)` | 1 | 二进制 → hex 字符串 |
| `crypto.hex_decode(hex)` | 1 | hex → 二进制 |
| `crypto.base64_encode(data)` | 1 | 二进制 → base64（RFC 4648） |
| `crypto.base64_decode(data)` | 1 | base64 → 二进制 |
| `crypto.aes_encrypt_ecb(data, key)` | 2 | AES-128-ECB 加密（数据 16 字节对齐） |
| `crypto.aes_decrypt_ecb(data, key)` | 2 | AES-128-ECB 解密 |
| `crypto.aes_encrypt_cbc(data, key, iv)` | 3 | AES-128-CBC 加密（PKCS#7 填充） |
| `crypto.aes_decrypt_cbc(data, key, iv)` | 3 | AES-128-CBC 解密 |
| `crypto.aes_encrypt_ctr(data, key, iv)` | 3 | AES-128-CTR 加密（流模式，无填充） |
| `crypto.aes_decrypt_ctr(data, key, iv)` | 3 | AES-128-CTR 解密 |
| `crypto.rc4(key, data)` | 2 | RC4 流密码（加密 = 解密） |
| `crypto.blowfish_encrypt(key, data)` | 2 | Blowfish ECB 加密 |
| `crypto.blowfish_decrypt(key, data)` | 2 | Blowfish ECB 解密 |
| `crypto.des_encrypt(key, data)` | 2 | DES ECB 加密（key ≥ 8 字节） |
| `crypto.des_decrypt(key, data)` | 2 | DES ECB 解密 |
| `crypto.triple_des_encrypt(key, data)` | 2 | 3DES ECB 加密（key ≥ 24 字节） |
| `crypto.triple_des_decrypt(key, data)` | 2 | 3DES ECB 解密 |

---

## CSV

**文件：** `csv/native_csv.h` · **注册：** `RegisterCsvLibraryApi`

| 函数 | 参数 | 说明 |
|------|------|------|
| `csv.decode(str, [sep])` | 1-2 | 解析 CSV 为行表；自动转换数字字段；默认分隔符 `,` |
| `csv.encode(rows, [sep])` | 1-2 | 编码行表为 CSV；自动引号特殊字段；默认分隔符 `,` |

---

## JSON

**文件：** `json/native_json.h` · **注册：** `RegisterJsonLibraryApi`

| 函数 | 参数 | 说明 |
|------|------|------|
| `json.encode(value)` | 1 | Lua 值 → JSON 字符串；连续整数键 1..N → 数组；浮点数用 `%.17g` |
| `json.decode(str)` | 1 | JSON 字符串 → Lua 值；`null` → `nil` |

---

## YAML

**文件：** `yaml/native_yaml.h` · **注册：** `RegisterYamlLibraryApi`

基于 [yaml-cpp](https://github.com/jbeder/yaml-cpp) 实现。类型自动推断：整数、浮点、布尔（`true/false/yes/no/on/off` 及其大小写变体）、`null`、字符串、数组（序列）、表（映射）。

| 函数 | 参数 | 说明 |
|------|------|------|
| `yaml.decode(str)` | 1 | YAML 字符串 → Lua 值 |
| `yaml.encode(value)` | 1 | Lua 值 → YAML 字符串 |

---

## TOML

**文件：** `toml/native_toml.h` · **注册：** `RegisterTomlLibraryApi`

基于 [toml++](https://github.com/marzer/tomlplusplus) 实现（header-only，C++17）。完整支持 TOML 类型：字符串、整数、浮点、布尔、日期/时间、数组、表。

| 函数 | 参数 | 说明 |
|------|------|------|
| `toml.decode(str)` | 1 | TOML 字符串 → Lua 值 |
| `toml.encode(value)` | 1 | Lua 值 → TOML 字符串（顶层需为表） |

---

## XML

**文件：** `xml/native_xml.h` · **注册：** `RegisterXmlLibraryApi`

基于 [pugixml](https://github.com/zeux/pugixml) 实现。

**解码约定：**
- 元素节点 → 表
- 属性 → `table["_attr_属性名"] = 值`
- 文本内容 → `table["_text"] = 值`
- 子元素按标签名分组，同名多个兄弟 → 数组
- 纯文本节点 → 直接返回字符串

| 函数 | 参数 | 说明 |
|------|------|------|
| `xml.decode(str)` | 1 | XML 字符串 → Lua 值（根元素表） |
| `xml.encode(value)` | 1 | Lua 值 → XML 字符串 |

---

## INI

**文件：** `ini/native_ini.h` · **注册：** `RegisterIniLibraryApi`

基于 [inih](https://github.com/benhoyt/inih) 实现（纯 C 单文件）。

**结构：** `ini.decode` 返回 `table[section][key] = value`。值自动推断数字/布尔/字符串。`ini.encode` 将顶层每个子表视为一个 `[section]`。

| 函数 | 参数 | 说明 |
|------|------|------|
| `ini.decode(str)` | 1 | INI 字符串 → Lua 值 |
| `ini.encode(value)` | 1 | Lua 值 → INI 字符串（顶层需为表，每个子表为一个 section） |

---

## MySQL

**文件：** `mysql/native_mysql.h`、`mysql/native_mysql_pool.h` · **注册：** `RegisterMysqlLibraryApi`、`RegisterMysqlPoolApi`

**`mysql.connect` 配置：** `{host, port, user, password, db}`

**`mysql_pool.create` 配置：** `{host, port, user, password, db, pool_size, timeout_ms, heartbeat_ms, max_retries}`

| 函数/方法 | 说明 |
|------|------|
| `mysql.connect(config, cb)` | 异步连接；回调 `function cb(err, conn)` |
| `mysql_pool.create(config)` | 创建连接池 |
| `conn:query(sql, cb)` | 异步查询；回调 `function cb(err, result)` |
| `conn:stmt_prepare(sql, cb)` | 预处理语句 |
| `conn:stmt_execute(id, params, cb)` | 执行预处理语句 |
| `conn:stmt_close(id)` | 关闭预处理语句 |
| `conn:tick()` | 泵网络事件 |
| `conn:close()` | 关闭连接 |
| `pool:acquire()` | 从池获取连接 |
| `pool:release(conn)` | 归还连接到池 |
| `pool:tick()` | 驱动心跳和重连 |
| `pool:close()` | 关闭连接池 |
| `pool:stats()` | 返回 `{total, healthy}` |

---

## SQLite

**文件：** `sqlite/native_sqlite.h` · **注册：** `RegisterSqliteLibraryApi`

| 函数/方法 | 说明 |
|------|------|
| `sqlite.open(filename)` | 打开/创建数据库，返回 db 对象 |
| `db:exec(sql)` | 执行 SQL；SELECT 返回行表，非 SELECT 返回 nil |
| `db:prepare(sql)` | 返回预处理语句对象 |
| `db:last_insert_rowid()` | 最近插入 rowid |
| `db:changes()` | 最近语句影响的行数 |
| `db:close()` | 关闭数据库 |
| `stmt:bind(...)` | 绑定参数（nil/int/float/string/bool） |
| `stmt:step()` | 执行一步；返回行表或 nil |
| `stmt:reset()` | 重置以重新执行 |
| `stmt:columns()` | 列名表 |
| `stmt:close()` | 销毁语句 |

> 所有操作均为同步，基于 SQLite3 amalgamation 源码。

---

## Serialize（序列化）

**文件：** `serialize/native_serialize.h` · **注册：** `RegisterSerializeLibraryApi`

| 函数 | 参数 | 说明 |
|------|------|------|
| `serialize.encode(value)` | 1 | Lua 值 → 二进制 wire 格式 |
| `serialize.decode(data)` | 1 | 二进制 wire 格式 → Lua 值 |

**编码方式：** 整数 zigzag + varint，浮点数小端 8 字节 memcpy，字符串去重（相同字符串第二次起存 varint 引用 ID），递归表序列化。

**支持类型：** `nil`、boolean、integer、float（二进制安全）、string（二进制安全）、table（嵌套）。表中不支持的类型会被跳过；顶层不支持的类型报错。

---

## Protobuf

**文件：** `protobuf/native_protobuf.h` · **注册：** `RegisterProtobufLibraryApi`

| 函数 | 参数 | 说明 |
|------|------|------|
| `protobuf.load(proto_text)` | 1 | 解析 proto3 文本，注册所有 message/enum |
| `protobuf.encode(name, table)` | 2 | Lua 表 → protobuf 二进制 |
| `protobuf.decode(name, data)` | 2 | protobuf 二进制 → Lua 表 |
| `protobuf.types()` | 0 | 已注册的 message 名称 |
| `protobuf.fields(name)` | 1 | 字段信息 `{name, number, type, type_name, label}` |

**支持的 proto3 特性：** message（嵌套）、enum、map\<K,V\>、oneof、repeated（默认 packed）、optional（显式存在）、全部 18 种标量类型、import（多文件）。

**Wire 格式：** tag = field_number << 3 | wire_type；整数 varint（sint 用 zigzag）；浮点数小端 memcpy；字符串/字节/消息长度前缀；packed repeated 标量默认。

---

## NativeObject（Lua 侧 API）

**文件：** `object/native_object.h` · **注册：** `RegisterNativeObjectApi`

| 函数 | 参数 | 说明 |
|------|------|------|
| `new_native_group()` | 0 | 创建组 arena，返回 `group_id` |
| `new_native_obj(group_id, type, id)` | 3 | 在组内创建对象 |
| `get_native_obj(type, id)` | 2 | 按 type+id 查找对象 |
| `del_native_group(group_id)` | 1 | 销毁组内所有对象，返回数量 |
| `new_global_obj(key, type)` | 2 | 创建全局对象（字符串键索引） |
| `get_global_obj(key)` | 1 | 查找全局对象 |
| `del_global_obj(key)` | 1 | 销毁全局对象 |

**NativeObject C++ API**（宿主侧绑定）：

| 方法 | 说明 |
|------|------|
| `RegisterMethod(name, lambda)` | 绑定 C++ 函数为 Lua 可调用方法 |
| `GetInt/SetInt/GetFloat/SetFloat/GetBool/SetBool/GetString/SetString` | 属性访问器 |
| `GetGroup/GetType/GetId` | 标识访问器 |
| `DestroyGroup(group_id)` | 批量销毁组内所有对象 |
| `SetFinalizer(fn)` | 设置清理回调 |

---

## 共享工具

**文件：** `native_common.h`

| 函数 | 说明 |
|------|------|
| `ThrowBadArgument(argno, fname, expected)` | 抛出标准化 "bad argument" 错误 |
| `CheckNumberArg(a, argno, fname)` | 拒绝非数字参数 |
| `CheckIntegerArg(a, argno, fname)` | Lua 5.4 对齐的整数检查；Int 通过，Float 必须无损整数 |
| `CheckStringArg(a, argno, fname)` | 拒绝非字符串参数 |
| `MakeIteratorClosure(state, fn, iter_state)` | 构造迭代器闭包用于 `pairs`/`ipairs`/`gmatch`/`file:lines` |
