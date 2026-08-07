#ifndef FAKELUA_NATIVE_STRING_H
#define FAKELUA_NATIVE_STRING_H

#include <string>
#include <string_view>

namespace fakelua {

class State;
struct CVar;

void RegisterStringLibraryApi(State *s);
std::string_view GetStringArgView(CVar a, std::string &temp);

// Helper: throw if arg is Bool or Table (not a string-coercible type), matching
// real Lua's luaL_checkstring behavior (numbers coerce, bool/table do not).
void CheckStringArg(const CVar &a, int argno, const char *fname);

}// namespace fakelua

#endif// FAKELUA_NATIVE_STRING_H
