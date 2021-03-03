#pragma once

// This file define interface, try to be consistent with the lua interface
class fakelua_state;

// Type for C functions registered with fakelua
typedef int (*fakelua_cfunction)(fakelua_state *L);

enum fakelua_error {
    FAKELUA_OK,
    FAKELUA_FILE_FAIL,
};

// get error code
extern "C" int fakelua_errno(fakelua_state *L);
// get error code string
extern "C" const char *fakelua_errstr(fakelua_state *L);

// create fake lua state
extern "C" fakelua_state *fakelua_newstate();
// close fake lua state
extern "C" void fakelua_close(fakelua_state *L);

// do lua file
extern "C" int fakelua_dofile(fakelua_state *L, const char *file_path);
// do lua string
extern "C" int fakelua_dostring(fakelua_state *L, const char *content);
