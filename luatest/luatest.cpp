#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>

extern "C"
{
#include "./lua/lua.h"
#include "./lua/lualib.h"
#include "./lua/lauxlib.h"
};
#include "lua_tinker.h"
#ifndef WIN32
#include "gperftools/profiler.h"
#endif

int cfunc1(int a, int b)
{
    return a - b;
}

class class1
{
public:
    class1()
    {
        b = 10;
    }
    int memfunc1(int a)
    {
        return a - b;
    }
private:
    int b;
};

int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        return 0;
    }

	lua_State * L = luaL_newstate();
	if (!L)
	{
		printf("lua_open error\n");
		return false;
	}

    lua_tinker::def(L, "cfunc1", cfunc1); 

	lua_tinker::dofile(L, argv[1]);

	int ret = 0;

	unsigned int begin = time(0);

#ifndef WIN32
	ProfilerStart("luatest.prof");
#endif

#ifndef _DEBUG
	for (int i = 0; i < 9000000; i++)
#else
	for (int i = 0; i < 1; i++)
#endif
	{
		ret = lua_tinker::call<int>(L, "myfunc1", 1, 2);
	}

#ifndef WIN32
	ProfilerStop();
#endif
	unsigned int end = time(0);

	printf("call ret %d %d\n", ret, end - begin);

	char c;
	std::cin>>c;

    return 0;
}

