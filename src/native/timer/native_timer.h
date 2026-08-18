#pragma once

#include "fakelua.h"

namespace fakelua::timer {

// 注册定时器库到 State：暴露 timer.create() 工厂函数
void RegisterTimerLibraryApi(State *s);

} // namespace fakelua::timer
