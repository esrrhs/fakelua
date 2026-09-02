#pragma once

#include "fakelua.h"

namespace fakelua::io {

void RegisterIoLibraryApi(State *s);

// Close FILE* handles belonging to this VM (stdin/stdout/stderr wrappers too,
// but the C FILE* themselves are never fclose'd).
void OnStateDeleted(State *s);

// Called by NativeObjectManager::Clear() before it destroys all objects.
// Clears the io wrapper/std caches so OnStateDeleted no longer holds stale pointers.
void OnNativeObjectManagerCleared();

}// namespace fakelua::io
