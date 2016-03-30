#pragma once

#include "types.h"
#include "hashmap.h"
#ifdef WIN32
#include <Windows.h>
#else
#include <sys/mman.h>
#endif

struct func_native
{
	String dump() const;
	// 名字
	const char * m_name;
	// 文件名
	const char * m_filename;
	// 包名
	const char * m_packagename;
	// code
	char * m_buff;
	size_t m_size;
	// 备份
	mutable func_native * m_backup;
};

#define FUNC_NATIVE_INI(fn) \
	memset(&(fn), 0, sizeof(func_native))

#ifdef WIN32
#define FUNC_NATIVE_CODE_FREE(fn) VirtualFree((fn).m_buff, 0, MEM_DECOMMIT | MEM_RELEASE);
#else
#define FUNC_NATIVE_CODE_FREE(fn) munmap((fn).m_buff, (fn).m_size);
#endif 

#define FUNC_NATIVE_DELETE(fn) \
	safe_fkfree(m_fk, (fn).m_name); \
	safe_fkfree(m_fk, (fn).m_filename); \
	safe_fkfree(m_fk, (fn).m_packagename); \
	FUNC_NATIVE_CODE_FREE(fn); \
	if ((fn).m_backup) \
	{ \
		safe_fkfree(m_fk, (fn).m_backup->m_name); \
		safe_fkfree(m_fk, (fn).m_backup->m_filename); \
		safe_fkfree(m_fk, (fn).m_backup->m_packagename); \
		FUNC_NATIVE_CODE_FREE(*((fn).m_backup)); \
	} \
	safe_fkfree(m_fk, (fn).m_backup)

class native
{
public:
	force_inline native(fake * fk) : m_fk(fk)
	{
	}
	force_inline ~native()
	{
		clear();
	}

	force_inline fake * getfake()
	{
		return m_fk;
	}

	force_inline void clear()
	{
	}

	bool add_func(const variant & name, const func_native & nt);
	
	String dump() const;
	
private:
	fake * m_fk;
};

