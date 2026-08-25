#pragma once

#include "state/state.h"

namespace fakelua::event {

// Register event library:
//   event.on(event_name, func_name) — subscribe
//   event.once(event_name, func_name) — subscribe once (auto-remove after fire)
//   event.off(event_name, func_name) — unsubscribe
//   event.emit(event_name, ...) — fire event, calls subscribers in order (vararg, up to 4 args)
//   event.clear(event_name) — remove all handlers for event
//   event.clear_all() — remove all handlers for all events
void RegisterEventLibraryApi(State *s);

// State 销毁时清掉该 VM 上的订阅，避免跨 State 串数据。
void OnStateDeleted(State *s);

}  // namespace fakelua::event
