#ifndef FAKELUA_NATIVE_STRING_H
#define FAKELUA_NATIVE_STRING_H

#include "native/native_common.h"
#include <string>
#include <string_view>

namespace fakelua {

class State;
struct CVar;

void RegisterStringLibraryApi(State *s);
std::string_view GetStringArgView(CVar a, std::string &temp);

}// namespace fakelua

#endif// FAKELUA_NATIVE_STRING_H
