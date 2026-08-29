#pragma once

#include "state/state.h"

namespace fakelua::log {

// Register Log library:
//   log.trace(msg, ...)
//   log.debug(msg, ...)
//   log.info(msg, ...)
//   log.warn(msg, ...)
//   log.error(msg, ...)
//   log.critical(msg, ...)
//   log.set_level(level)       -- 设置全局日志级别 (0-6)
//   log.set_file(path)         -- 设置日志文件路径
void RegisterLogLibraryApi(State *s);

}  // namespace fakelua::log
