#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include "fakescript.h"
#include <string.h>
#include <vector>
#include <algorithm>
#ifdef WIN32
#else
#include "gperftools/profiler.h"
#endif

// for test
int test_cfunc1(int a, int b)
{
	return a - b;
}

// for test
class test_class1
{
public:
	test_class1()
	{
		b = 10;
	}
	int test_memfunc1(int a)
	{
		return a - b;
	}
private:
	int b;
};

// for test
test_class1 * new_test_class1()
{
	return new test_class1();
}

void error_log(fake * fk, int eno, const char * file, int lineno, const char * func, const char * str)
{
	printf("fake error[%d] in file(%s:%d) func(%s) : ", eno, file, lineno, func);
	printf("%s\n", str);
	printf("call stack :\n%s\n", fkgetcurcallstack(fk));
	printf("cur routine :\n%s\n", fkgetcurroutine(fk));
}

static const char * g_replacebuff = "start compile";

// 函数
bool readfile(const char * filename, int addsize, char * & buf, int & filesize);
bool writefile(const char * filename, const char * buf, int filesize);
int parsearg(fake * fk, int & argc, const char ** & argv);
int startdebug(fake * fk);


// 开关选项
bool g_isopenprofile = false;
bool g_isopenjit = false;
bool g_isopengoogleprofile = false;
int g_testnum = 1;
bool g_issave = false;
bool g_isload = false;
bool g_isprintoutput = false;
bool g_iscompile = false;
bool g_iscompiled = false;
bool g_isstep = false;
const char * g_selfname = 0;

int main(int argc, const char *argv[])
{
	fake * fk = newfake();

	// 解析参数
	int parseret = parsearg(fk, argc, argv);
	if (parseret != 0)
	{
		return parseret;
	}

	if (g_iscompiled)
	{
		fksetargv(fk, argc - 1, argv + 1);
	}
	else
	{
		fksetargv(fk, argc - 2, argv + 2);
	}
	fkopenalllib(fk);
	fkseterrorfunc(fk, error_log);

	// for test
	fkreg(fk, "test_cfunc1", test_cfunc1);
	fkreg(fk, "new_test_class1", new_test_class1);
	test_class1 c1;
	fkreg(fk, "test_memfunc1", &test_class1::test_memfunc1); 

	// 编译后文件
	if (g_iscompiled)
	{
		// 打开自己
		char * selftmpbuf = 0;
		int selfsize = 0;
		if (!readfile(g_selfname, 0, selftmpbuf, selfsize))
		{
			return -1;
		}

		int startpos = 0;
		int size = 0;
		memcpy(&startpos, g_replacebuff, sizeof(startpos));
		memcpy(&size, g_replacebuff + 4, sizeof(size));

		if (fkloadfunc(fk, selftmpbuf + startpos, size) == -1)
		{
			printf("load func fail\n");
			return -1;
		}

		free(selftmpbuf);
	}
	// 解析文件
	else if (!g_isload)
	{
		fkparse(fk, argv[1]);
		if (fkerror(fk))
		{
			return -1;
		}
	}
	// 读文件
	else
	{
		// 读文件
		char * tmpbuf = 0;
		int size = 0;
		if (!readfile(argv[1], 0, tmpbuf, size))
		{
			return -1;
		}

		if (fkloadfunc(fk, tmpbuf, size) == -1)
		{
			printf("load func fail\n");
			return -1;
		}

		free(tmpbuf);
		printf("load from %s ok, size %d\n", argv[1], size);
	}

	// 存文件
	if (g_issave)
	{
		int tmpsize = 1024 * 1024;
		char * tmpbuf = (char *)malloc(tmpsize);
		int size = fksavefunc(fk, tmpbuf, tmpsize);
		if (size == -1)
		{
			printf("save func fail\n");
			return -1;
		}

		const char * filename = "fake.bin";
		if (!writefile(filename, tmpbuf, size))
		{
			return -1;
		}
		free(tmpbuf);
		
		printf("save to %s ok, size %d\n", filename, size);
		
		return 0;
	}

	// 编译到一起
	if (g_iscompile)
	{
		int tmpsize = 1024 * 1024;
		char * tmpbuf = (char *)malloc(tmpsize);
		int size = fksavefunc(fk, tmpbuf, tmpsize);
		if (size == -1)
		{
			printf("compile func fail\n");
			return -1;
		}

		// 打开自己
		char * selftmpbuf = 0;
		int selfsize = 0;
		if (!readfile(g_selfname, size, selftmpbuf, selfsize))
		{
			return -1;
		}

		// 替换内存中的字符串
		bool isfind = false;
		for (int i = 0; i < selfsize; i++)
		{
			if (memcmp(selftmpbuf + i, g_replacebuff, strlen(g_replacebuff)) == 0)
			{
				memcpy(selftmpbuf + i, &selfsize, sizeof(selfsize));
				memcpy(selftmpbuf + i + 4, &size, sizeof(size));
				isfind = true;
				break;
			}
		}
		if (!isfind)
		{
			printf("replace %s pos fail\n", argv[0]);
			return -1;
		}

		// 复制后面的
		memcpy(selftmpbuf + selfsize, tmpbuf, size);
		free(tmpbuf);

		// 输出
		const char * filename = "out.exe";
		if (!writefile(filename, selftmpbuf, selfsize + size))
		{
			return -1;
		}
		free(selftmpbuf);

		printf("compile to %s ok, size %d\n", filename, selfsize + size);

		return 0;
	}

#ifndef WIN32
	if (g_isopengoogleprofile)
	{
		ProfilerStart("fake.prof");
	}
#endif

	uint32_t begintime = time(0);
	int fkret = 0;

	// run
	for (int i = 0; i < g_testnum; i++)
	{
		if (!g_isopenjit)
		{
			if (g_isstep)
			{
				fkret = startdebug(fk);
			}
			else
			{
				fkret = fkrun<int>(fk, "main");
			}
		}
		else
		{
			fkret = fkrunjit<int>(fk, "main");
		}
	}
	
	uint32_t endtime = time(0);

#ifndef WIN32
	if (g_isopengoogleprofile)
	{
		ProfilerStop();
	}
#endif

	if (fkerror(fk))
	{
		return -1;
	}

	if (g_isopenprofile)
	{
		printf("\n%s", fkdumpprofile(fk));
	}

	if (g_isprintoutput)
	{
		printf("main return : %d, use time %d\n", fkret, endtime - begintime);
	}

	delfake(fk);

	return 0;
}

bool readfile(const char * filename, int addsize, char * & buf, int & filesize)
{
	// 打开
	FILE * fp = fopen(filename, "rb");
	if (!fp)
	{
		printf("open %s for read fail\n", filename);
		return false;
	}

	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char * tmpbuf = (char *)malloc(size + addsize);
	if (fread(tmpbuf, size, 1, fp) != 1)
	{
		printf("read %s fail\n", filename);
		return false;
	}
	fclose(fp);

	buf = tmpbuf;
	filesize = size;

	return true;
}

bool writefile(const char * filename, const char * buf, int filesize)
{
	FILE * fp = fopen(filename, "wb");
	if (!fp)
	{
		printf("open %s for write fail\n", filename);
		return false;
	}

	if (fwrite(buf, filesize, 1, fp) != 1)
	{
		printf("write %s fail\n", filename);
		return -1;
	}

	fclose(fp);

	return true;
}

int parsearg(fake * fk, int & argc, const char ** & argv)
{
	g_selfname = argv[0];

	// start compile，memcmp无解只能用这种
	if (g_replacebuff[0] != 's' ||
		g_replacebuff[1] != 't' ||
		g_replacebuff[2] != 'a' ||
		g_replacebuff[3] != 'r' ||
		g_replacebuff[4] != 't' ||
		g_replacebuff[5] != ' ' ||
		g_replacebuff[6] != 'c' ||
		g_replacebuff[7] != 'o' ||
		g_replacebuff[8] != 'm' ||
		g_replacebuff[9] != 'p' ||
		g_replacebuff[10] != 'i' ||
		g_replacebuff[11] != 'l' ||
		g_replacebuff[12] != 'e')
	{
		g_iscompiled = true;
	}
	else if (argc < 2)
	{
		printf("try -h for help\n");
		return -1;
	}

	if (argc >= 2 && argv[1][0] == '-')
	{
		if (strstr(argv[1], "h"))
		{
			printf("\
fakescript\n\narg : [-hjpgtsl] file [arg]\n\
-h help\n\
-j open jit\n\
-p open profile\n\
-g open google profile\n\
-t open test mode\n\
-s save func to fake.bin\n\
-l load func from file\n\
-o print return value and time\n\
-c compile script to out.exe\n\
-d debug step mod\n\
sample:./fake a.fk\n\n");
			return -1;
		}

		if (strstr(argv[1], "j"))
		{
			fkopenjit(fk);
			g_isopenjit = true;
		}

		if (strstr(argv[1], "p"))
		{
			fkopenprofile(fk);
			g_isopenprofile = true;
		}

#ifndef WIN32
		if (strstr(argv[1], "g"))
		{
			g_isopengoogleprofile = true;
		}
#endif

		if (strstr(argv[1], "t"))
		{
			g_testnum = 9000000;
		}

		if (strstr(argv[1], "s"))
		{
			g_issave = true;
		}

		if (strstr(argv[1], "l"))
		{
			g_isload = true;
		}

		if (strstr(argv[1], "o"))
		{
			g_isprintoutput = true;
		}

		if (strstr(argv[1], "c"))
		{
			printf("%s\n", g_replacebuff);
			g_iscompile = true;
		}

		if (strstr(argv[1], "d"))
		{
			g_isstep = true;
		}

		argc--;
		argv++;
	}

	return 0;
}

enum debug_command
{
	debug_next,
	debug_step,
	debug_next_instruction,
	debug_step_instruction,
	debug_continue,
	debug_breakpoint,
	debug_enable,
	debug_disable,
	debug_delete,
	debug_info,
	debug_finish,
	debug_list,
	debug_print,
	debug_set,
	debug_watch,
	debug_backtrace,
	debug_frame,
	debug_disa,
	debug_routine,
};

struct breakpoint
{
	bool enable;
	int id;
	std::string file;
	int line;
};

typedef std::vector<breakpoint> breakpoint_list;

void show_debug_code(fake * fk, int rid, int frame, int range)
{
	int curline = fkgetcurlinebyroutinebyframe(fk, rid, frame);
	for (int i = curline - range; i <= curline + range; i++)
	{
		if (i > 0)
		{
			const char * code = fkgetfilecode(fk, fkgetcurfilebyroutinebyframe(fk, rid, frame), i);
			if (code)
			{
				printf("%s%d\t%s", curline == i ? "*" : "", i, code);
			}
		}
	}
	printf("\n");
}

void show_watch_variant(fake * fk, int rid, int frame, std::vector<std::string> & watchvec)
{
	for (int i = 0; i < (int)watchvec.size(); i++)
	{
		printf("%s\n", fkgetcurvaiantbyroutinebyframe(fk, rid, frame, watchvec[i].c_str()));
	}
}

void check_show_func_header(fake * fk, int rid, int frame, int & lastrid, const char * & lastfunc)
{
	const char * curfunc = fkgetcurfuncbyroutinebyframe(fk, rid, frame);
	if (rid != lastrid)
	{
		printf("routine %s\n", fkgetcurroutinebyid(fk, rid));
		lastrid = rid;
	}
	if (curfunc != lastfunc || strcmp(curfunc, lastfunc) != 0)
	{	
		printf("file %s, line %d, func %s\n", 
			fkgetcurfilebyroutinebyframe(fk, rid, frame), 
			fkgetcurlinebyroutinebyframe(fk, rid, frame), 
			curfunc);
		lastfunc = curfunc;
	}
}

bool check_trigger_breakpoint(fake * fk, breakpoint_list & blist)
{
	std::string curfile = fkgetcurfile(fk);
	if ((int)curfile.find_last_of('/') != -1)
	{
		curfile = curfile.substr(curfile.find_last_of('/') + 1);
	}
	int line = fkgetcurline(fk);
	for (int i = 0;  i < (int)blist.size(); i++)
	{
		breakpoint & tmp = blist[i];
		if (tmp.enable && tmp.line == line && tmp.file == curfile)
		{
			printf("Trigger Breakpoint %d at file %s, line %d\n", tmp.id, tmp.file.c_str(), tmp.line);
			return true;
		}
	}
	
	return false;
}

void show_debug_help(fake * fk)
{
	printf("h help\n\
n\tnext\n\
s\tstep\n\
ni\tnext bytecode\n\
si\tstep bytecode\n\
c\tcontinue\n\
l\tlist\n\
p\tprint\n\
set\tset\n\
wa\twatch\n\
b\tbreakpoint\n\
en\tenable\n\
dis\tdisable\n\
d\tdelete\n\
i\tinfo\n\
bt\tbacktrace\n\
f\tframe\n\
fin\tfinish\n\
r\troutine\n\
disa\tview bytecode\n");
}

void get_debug_command(fake * fk, int & command, std::vector<std::string> & paramvec)
{
	while (1)
	{
		char buff[100];
		printf("(fake) ");
		fgets(buff, sizeof(buff), stdin);
		std::remove(buff, buff + 100, '\n');
		std::remove(buff, buff + 100, '\r');
		if (!strlen(buff))
		{
			if (command == -1)
			{
				continue;
			}
			return;
		}

		paramvec.clear();
		char * p = strtok(buff, " "); 
		while(p) 
		{
			paramvec.push_back(p);
			p = strtok(0, " ");
		}
		
		if (paramvec.empty())
		{
			return;
		}
		
		std::string strcommand = paramvec[0];
		paramvec.erase(paramvec.begin());
			
		if (strcommand == "n")
		{
			command = debug_next;
		}
		else if (strcommand == "s")
		{
			command = debug_step;
		}
		else if (strcommand == "ni")
		{
			command = debug_next_instruction;
		}
		else if (strcommand == "si")
		{
			command = debug_step_instruction;
		}
		else if (strcommand == "c")
		{
			command = debug_continue;
		}
		else if (strcommand == "b")
		{
			command = debug_breakpoint;
		}
		else if (strcommand == "dis")
		{
			command = debug_disable;
		}
		else if (strcommand == "en")
		{
			command = debug_enable;
		}
		else if (strcommand == "d")
		{
			command = debug_delete;
		}
		else if (strcommand == "i")
		{
			command = debug_info;
		}
		else if (strcommand == "fin")
		{
			command = debug_finish;
		}
		else if (strcommand == "l")
		{
			command = debug_list;
		}
		else if (strcommand == "wa")
		{
			command = debug_watch;
		}
		else if (strcommand == "p")
		{
			command = debug_print;
		}
		else if (strcommand == "set")
		{
			command = debug_set;
		}
		else if (strcommand == "bt")
		{
			command = debug_backtrace;
		}
		else if (strcommand == "f")
		{
			command = debug_frame;
		}
		else if (strcommand == "disa")
		{
			command = debug_disa;
		}
		else if (strcommand == "r")
		{
			command = debug_routine;
		}
		else if (strcommand == "h")
		{
			show_debug_help(fk);
			continue;
		}
		else
		{
			printf("use h to get help\n");
			continue;
		}
		
		return;
	}
}

int startdebug(fake * fk)
{
	int fkret = 0;
	fkopenstepmod(fk);
	fkrun<int>(fk, "main");
	bool isend = false;
	int range = 0;
	int command = debug_next;
	std::vector<std::string> paramvec;
	const char * lastfunc = 0;
	breakpoint_list blist;
	int bindex = 0;
	int frame = 0;
	int lastrid = fkgetcurroutineid(fk);
	int rid = lastrid;
	std::vector<std::string> watchvec;
	bool firsttime = true;
	while (1)
	{
		show_watch_variant(fk, rid, frame, watchvec);
		check_show_func_header(fk, rid, frame, lastrid, lastfunc);
		show_debug_code(fk, rid, frame, range);
INPUT:
		get_debug_command(fk, command, paramvec);
		switch (command)
		{
		case debug_next:
			{		
				frame = 0;
				rid = fkgetcurroutineid(fk);
						
				if (firsttime)
				{
					if (check_trigger_breakpoint(fk, blist))
					{
						firsttime = false;
						break;
					}
				}
				firsttime = false;
				
				const char * lastfile = fkgetcurfile(fk);
				int lastline = fkgetcurline(fk);
				int laststacklength = fkgetcurcallstacklength(fk);
				int lastrid = rid;
				const char * curfile = fkgetcurfile(fk);
				int curline = fkgetcurline(fk);
				int curstacklength = fkgetcurcallstacklength(fk);
				int currid = rid;
				while (currid == lastrid && 
					(curstacklength > laststacklength || 
					(strcmp(lastfile, curfile) == 0 && lastline == curline)))
				{
					fkret = fkresume<int>(fk, isend);
					if (isend)
					{
						break;
					}
					curfile = fkgetcurfile(fk);
					curline = fkgetcurline(fk);
					curstacklength = fkgetcurcallstacklength(fk);
					currid = fkgetcurroutineid(fk);
					if (check_trigger_breakpoint(fk, blist))
					{
						break;
					}
				}
				
				rid = fkgetcurroutineid(fk);
			}
			break;
		case debug_step:
			{
				frame = 0;
				rid = fkgetcurroutineid(fk);
				
				if (firsttime)
				{
					if (check_trigger_breakpoint(fk, blist))
					{
						firsttime = false;
						break;
					}
				}
				firsttime = false;
				
				const char * lastfile = fkgetcurfile(fk);
				int lastline = fkgetcurline(fk);
				int lastrid = rid;
				const char * curfile = fkgetcurfile(fk);
				int curline = fkgetcurline(fk);
				int currid = rid;
				while (currid == lastrid &&
					(strcmp(lastfile, curfile) == 0 && lastline == curline))
				{
					fkret = fkresume<int>(fk, isend);
					if (isend)
					{
						break;
					}
					curfile = fkgetcurfile(fk);
					curline = fkgetcurline(fk);
					currid = fkgetcurroutineid(fk);
					if (check_trigger_breakpoint(fk, blist))
					{
						break;
					}
				}
				
				rid = fkgetcurroutineid(fk);
			}
			break;
		case debug_next_instruction:
			{
				frame = 0;
				rid = fkgetcurroutineid(fk);
				
				if (firsttime)
				{
					if (check_trigger_breakpoint(fk, blist))
					{
						firsttime = false;
						break;
					}
				}
				firsttime = false;
				
				int laststacklength = fkgetcurcallstacklength(fk);
				int lastrid = rid;
				int curstacklength = fkgetcurcallstacklength(fk);
				int currid = rid;
				do
				{
					fkret = fkresume<int>(fk, isend);
					if (isend)
					{
						break;
					}
					curstacklength = fkgetcurcallstacklength(fk);
					currid = fkgetcurroutineid(fk);
					if (check_trigger_breakpoint(fk, blist))
					{
						break;
					}
				}
				while (currid == lastrid &&
					curstacklength > laststacklength);
					
				rid = fkgetcurroutineid(fk);
			}
			break;
		case debug_step_instruction:
			{
				frame = 0;
				rid = fkgetcurroutineid(fk);
				
				if (firsttime)
				{
					if (check_trigger_breakpoint(fk, blist))
					{
						firsttime = false;
						break;
					}
				}
				firsttime = false;
				
				fkret = fkresume<int>(fk, isend);
				
				rid = fkgetcurroutineid(fk);
				if (check_trigger_breakpoint(fk, blist))
				{
					break;
				}
			}
			break;
		case debug_continue:
			{
				frame = 0;
				rid = fkgetcurroutineid(fk);

				if (firsttime)
				{
					if (check_trigger_breakpoint(fk, blist))
					{
						firsttime = false;
						break;
					}
				}
				firsttime = false;
				
				while (1)
				{
					fkret = fkresume<int>(fk, isend);
					rid = fkgetcurroutineid(fk);
					if (check_trigger_breakpoint(fk, blist))
					{
						break;
					}
					if (isend)
					{
						break;
					}
				}
			}
			break;
		case debug_breakpoint:
			{
				breakpoint tmp;
				if (paramvec.empty())
				{
					tmp.file = fkgetcurfilebyroutinebyframe(fk, rid, frame);
					tmp.line = fkgetcurlinebyroutinebyframe(fk, rid, frame);
				}
				else
				{
					std::string str = paramvec[0];
					int subpos = str.find(':');
					if (subpos != -1)
					{
						std::string filestr = str.substr(0, subpos);
						std::string linestr = str.substr(subpos + 1);
						
						tmp.file = filestr;
						tmp.line = atoi(linestr.c_str());
					}
					else
					{
						bool isnumber = true;
						for (int i = 0; i < (int)str.size(); i++)
						{
							if (!isdigit(str[i]))
							{
								isnumber = false;
								break;
							}
						}

						if (isnumber)
						{
							tmp.file = fkgetcurfilebyroutinebyframe(fk, rid, frame);
							tmp.line = atoi(str.c_str());
						}
						else 
						{
							if (!fkisfunc(fk, str.c_str()))
							{
								printf("%s is not func\n", str.c_str());
								continue;
							}
							
							tmp.file = fkgetfuncfile(fk, str.c_str());
							tmp.line = fkgetfuncstartline(fk, str.c_str());
						}
					}
				}
				tmp.enable = true;
				tmp.id = bindex;
				
				if ((int)tmp.file.find_last_of('/') != -1)
				{
					tmp.file = tmp.file.substr(tmp.file.find_last_of('/') + 1);
				}
				
				blist.push_back(tmp);
				bindex++;
				printf("Breakpoint %d at file %s, line %d total %d\n", tmp.id, tmp.file.c_str(), tmp.line, (int)blist.size());
			}
			break;
		case debug_enable:
			{
				if (paramvec.empty())
				{
					for (int i = 0;  i < (int)blist.size(); i++)
					{
						breakpoint & tmp = blist[i];
						tmp.enable = true;
					}
				}
				else
				{
					int id = atoi(paramvec[0].c_str());
					for (int i = 0;  i < (int)blist.size(); i++)
					{
						breakpoint & tmp = blist[i];
						if (tmp.id == id)
						{
							tmp.enable = true;
						}
					}
				}
			}
			break;
		case debug_disable:
			{
				if (paramvec.empty())
				{
					for (int i = 0;  i < (int)blist.size(); i++)
					{
						breakpoint & tmp = blist[i];
						tmp.enable = false;
					}
				}
				else
				{
					int id = atoi(paramvec[0].c_str());
					for (int i = 0;  i < (int)blist.size(); i++)
					{
						breakpoint & tmp = blist[i];
						if (tmp.id == id)
						{
							tmp.enable = false;
						}
					}
				}
			}
			break;
		case debug_delete:
			{
				if (paramvec.empty())
				{
					blist.clear();
					watchvec.clear();
					continue;
				}
				else
				{
					int id = atoi(paramvec[0].c_str());
					for (int i = 0;  i < (int)blist.size(); i++)
					{
						breakpoint & tmp = blist[i];
						if (tmp.id == id)
						{
							blist.erase(blist.begin() + i);
						}
					}
				}
			}
			break;
		case debug_info:
			{
				if (paramvec.empty())
				{
					printf("need arg, useage: i b, i r\n");
					continue;
				}

				if (paramvec[0] == "b")
				{
					printf("Id\tEnb\twhere\n");
					for (int i = 0;  i < (int)blist.size(); i++)
					{
						breakpoint & tmp = blist[i];
						printf("%d\t%s\tfile %s, line %d\n", tmp.id, tmp.enable ? "y" : "n", tmp.file.c_str(), tmp.line);
					}
				}
				else if (paramvec[0] == "r")
				{
					for (int i = 0;  i < (int)fkgetcurroutinenum(fk); i++)
					{
						printf("%s%s%s\n", fkgetroutineidbyindex(fk, i) == rid ? "*" : "", 
							fkgetroutineidbyindex(fk, i) == fkgetcurroutineid(fk) ? "->" : "", 
							fkgetcurroutinebyindex(fk, i));
					}
				}
				
				printf("\n");
			}
			break;
		case debug_finish:
			{
				frame = 0;
				rid = fkgetcurroutineid(fk);
				
				int laststacklength = fkgetcurcallstacklength(fk);
				int curstacklength = fkgetcurcallstacklength(fk);
				do
				{
					fkret = fkresume<int>(fk, isend);
					if (isend)
					{
						break;
					}
					curstacklength = fkgetcurcallstacklength(fk);
				}
				while (curstacklength >= laststacklength);
			}
			break;
		case debug_list:
			{
				int listrange = 3;
				if (!paramvec.empty())
				{
					listrange = atoi(paramvec[0].c_str());
				}
				show_debug_code(fk, rid, frame, listrange);
				goto INPUT;
			}
			break;
		case debug_print:
			{
				if (paramvec.empty())
				{
					printf("need arg, useage: p variant\n");
					continue;
				}

				std::string name = paramvec[0];
				printf("%s\n", fkgetcurvaiantbyroutinebyframe(fk, rid, frame, name.c_str()));
			}
			break;
		case debug_set:
			{
				if (paramvec.size() < 2)
				{
					printf("need arg, useage: s variant value\n");
					continue;
				}

				std::string name = paramvec[0];
				std::string value = paramvec[1];
				fksetcurvaiantbyroutinebyframe(fk, rid, frame, name.c_str(), value.c_str());
				printf("%s\n", fkgetcurvaiantbyroutinebyframe(fk, rid, frame, name.c_str()));
			}
			break;
		case debug_watch:
			{
				if (paramvec.empty())
				{
					printf("need arg, useage: wa variant\n");
					continue;
				}

				std::string name = paramvec[0];
				watchvec.push_back(name);
			}
			break;
		case debug_backtrace:
			{
				int length = fkgetcurcallstacklengthbyroutine(fk, rid);
				for (int i = 0; i < length; i++)
				{
					printf("%s%s\n", i == frame ? "*" : " ", fkgetcurcallstackbyroutinebyframe(fk, rid, i));
				}
				goto INPUT;
			}
			break;
		case debug_frame:
			{
				if (paramvec.empty())
				{
					frame = 0;
				}
				else
				{
					int theframe = atoi(paramvec[0].c_str());
					if (theframe < 0 || theframe >= fkgetcurcallstacklengthbyroutine(fk, rid))
					{
						printf("%d is invalid\n", theframe);
					}
					frame = theframe;
				}
			}
			break;
		case debug_disa:
			{
				int pos = fkgetcurbytecodeposbyroutine(fk, rid);
				const char * func = fkgetcurfuncbyroutinebyframe(fk, rid, frame);
				printf("%s\n", fkdumpfunc(fk, func, pos));
			}
			break;
		case debug_routine:
			{
				if (paramvec.empty())
				{
					printf("need arg, useage: r rid\n");
					continue;
				}

				int id = atoi(paramvec[0].c_str());
				if (!fkishaveroutine(fk, id))
				{
					printf("no routine %d\n", id);
					continue;
				}
				
				rid = id;
			}
			break;
		default:
			continue;
		}
		if (isend)
		{
			break;
		}
	}
	printf("end\n");
	return fkret;
}

