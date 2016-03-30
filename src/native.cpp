#include "native.h"
#include "fake.h"
#include "buffer.h"

String func_native::dump() const
{
	String ret;

	// Ãû×Ö
	ret += "\n[";
	ret += m_name;
	ret += "]\n";

	ret += "\n\t////// code ";
	ret += fkitoa(m_size);
	ret += " //////";
	// code
	for (int i = 0; i < (int)m_size; i++)
	{
		if (i % 8 == 0)
		{
			ret += "\n\t[";
			ret += fkitoa(i);
			ret += "]\t";
		}
		unsigned char c = m_buff[i];
		ret += fkxtoa(c, 2);
		ret += ", ";
	}
	ret += "\n\n";
	return ret;
}

String native::dump() const
{
	String ret;
	for (const fkhashmap<variant, funcunion>::ele * p = m_fk->fm.m_shh.first(); p != 0; p = m_fk->fm.m_shh.next())
	{
		const funcunion & f = *p->t;
		if (f.havefn)
		{
			ret += f.fn.dump();
		}
	}
	return ret;
}

bool native::add_func(const variant & name, const func_native & nt)
{
	m_fk->fm.add_func_native(name, nt);

	FKLOG("add func native %s", vartostring(&name).c_str());

	return true;
}

