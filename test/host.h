#pragma once

#include "fake-inc.h"

// Interpreter with packages and the C/C++ helpers the sample scripts call.
fake *fk_test_new();
void fk_bind_sample_host(fake *fk);

int test_cfunc1(int a, int b);
