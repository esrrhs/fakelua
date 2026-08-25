#pragma once

#include "fakelua.h"

namespace fakelua::timer {

// 注册定时器库到 State：暴露 timer.create() 工厂函数
void RegisterTimerLibraryApi(State *s);

// State 销毁时清掉该 VM 上的定时器，避免跨 State 串数据。
void OnStateDeleted(State *s);

} // namespace fakelua::timer
