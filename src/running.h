#pragma once

#include "types.h"

struct running
{
	running(fake * fk) : m_fk(fk), rundeps(0), curroutine(0)
	{
	}
	~running()
	{
		clear();
	}
	void clear()
	{
		rundeps = 0;
		curroutine = 0;
		curcallstack.clear();
	}

	fake * m_fk;
	// 执行迭代计数
	int rundeps;
	// 当前执行的routine
	routine * curroutine;
	// call stack
	String curcallstack;
};

