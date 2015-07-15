#pragma once

#include "types.h"
#include "routine.h"
#include "array.h"
#include "pool.h"

struct fake;
struct processor
{
public:

	routine * start_routine(const variant & func, int retnum, int * retpos);
	
	void run();

public:
	fake * m_fk;
	poollist<routine> m_pl;
	uint32_t m_routine_num;
};

#define PROCESS_INI(pro, fk) (pro).m_fk = fk;\
	POOLLIST_INI((pro).m_pl, fk)

#define PROCESS_DELETE(pro) \
		pool<routine>::node * prn = (pro).m_pl.p.m_data;\
		while (prn != 0)\
		{\
			ROUTINE_DELETE(prn->t);\
			prn = prn->next;\
		}\
		POOLLIST_DELETE((pro).m_pl)

#define PROCESS_CLEAR(pro) POOLLIST_CLEAR((pro).m_pl, routine, USE(n));\
	(pro).m_routine_num = 0

