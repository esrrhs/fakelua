#pragma once

#include <string>
#include <list>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <typeinfo>
#include <stdio.h>
#include <algorithm>
#include "types.h"
#include "myflexer.h"
#include "fakescript.h"
#include "compiler.h"
#include "binary.h"
#include "paramstack.h"
#include "interpreter.h"
#include "native.h"
#include "assembler.h"
#include "machine.h"
#include "stringheap.h"
#include "bindfunc.h"
#include "buildinfunc.h"
#include "profile.h"
#include "pool.h"
#include "processor.h"
#include "container.h"
#include "funcmap.h"
#include "running.h"
#include "parser.h"

struct fake
{
	fake() : errorno(0), errorcb(0), pa(this), bbin(this), bin(this), nt(this), as(this, &nt), sh(this), mac(this), bf(this), bif(this), pf(this), con(this), fm(this), rn(this)
    {
        POOL_INI(pp, this);
    }
    ~fake()
    {
        clear();
        pool<processor>::node * ppn = pp.m_data;
        while (ppn != 0)
        {
            PROCESS_DELETE(ppn->t);
            ppn = ppn->next;
        }
        POOL_DELETE(pp);
    }

    // 清空
    void clear()
    {
        clearerr();
        pa.clear();
        bbin.clear();
        bin.clear();
        nt.clear();
        as.clear();
        ps.clear();
        POOL_CLEAR(pp);
        sh.clear();
        mac.clear();
        bf.clear();
        bif.clear();
        pf.clear();
		con.clear();
		fm.clear();
		rn.clear();
    }
    
    void clearerr()
    {
        errorno = 0;
        errorstr[0] = 0;
    }
    
    int errorno;
    char errorstr[128];
    fkerrorcb errorcb;
    
    // 配置
    fakeconfig cfg;

    // 解析
    parser pa;

    // 缓存二进制
    backupbinary bbin;

    // 二进制
    binary bin;
    
    // 本地jit代码
    native nt;

    // 汇编器
    assembler as;

    // c的参数栈
    paramstack ps;

    // 执行器池子
    pool<processor> pp;

    // 字符串集合
    stringheap sh;

    // 当前线程本地环境
    machine mac;

    // 绑定C函数集合
    bindfunc bf;

    // 内建的函数集合
    buildinfunc bif;

    // 性能检测
    profile pf;

    // 容器
    container con;

	// 函数索引
	funcmap fm;

	// 当前运行状态
	running rn;
};

template <typename T>
T * fknew(fake * fk)
{
    T * t = (T *)fk->cfg.fkm(sizeof(T));
    new (t) T();
    return t;
}

template <typename T, typename P1>
T * fknew(fake * fk, P1 p1)
{
    T * t = (T *)fk->cfg.fkm(sizeof(T));
	if (!t)
	{
		return 0;
	}
    new (t) T(p1);
    return t;
}

template <typename T>
void fkdelete(fake * fk, T * p)
{
	if (p)
	{
		p->~T();
		fk->cfg.fkf(p);
	}
}

