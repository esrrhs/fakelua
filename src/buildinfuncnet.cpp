#include "buildinfuncnet.h"
#include "fake.h"
#include "fakescript.h"
#include "buffer.h"
#ifdef WIN32
#include <windows.h>
#include <winsock.h>
#else
#include <sys/types.h>		  /* See NOTES */
#include <sys/socket.h>
#include <execinfo.h>
#include <signal.h>
#include <exception>
#include <setjmp.h>
#include <pthread.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <linux/limits.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <malloc.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/syscall.h> 
#include <sys/reg.h>
#include <sys/user.h>
#endif
#include "selector.h"

// buffer
void buildin_buffer_new(fake * fk, interpreter * inter)
{
	int size = fkpspop<int>(fk);
	buffer * b = fk->bif.newbuffer(size);
	fkpspush<buffer *>(fk, b);
}
void buildin_buffer_clear(fake * fk, interpreter * inter)
{
	buffer * b = fkpspop<buffer *>(fk);
	if (b)
	{
		b->clear();
	}
	fkpspush<int>(fk, 1);
}
void buildin_buffer_maxsize(fake * fk, interpreter * inter)
{
	buffer * b = fkpspop<buffer *>(fk);
	if (b)
	{
		fkpspush<int>(fk, b->capacity());
	}
	else
	{
		fkpspush<int>(fk, 0);
	}
}
void buildin_buffer_size(fake * fk, interpreter * inter)
{
	buffer * b = fkpspop<buffer *>(fk);
	if (b)
	{
		fkpspush<int>(fk, b->size());
	}
	else
	{
		fkpspush<int>(fk, 0);
	}
}
void buildin_buffer_store(fake * fk, interpreter * inter)
{
	buffer * b = fkpspop<buffer *>(fk);
	if (b)
	{
		b->store();
	}
	fkpspush<int>(fk, 1);
}
void buildin_buffer_restore(fake * fk, interpreter * inter)
{
	buffer * b = fkpspop<buffer *>(fk);
	if (b)
	{
		b->restore();
	}
	fkpspush<int>(fk, 1);
}
void buildin_buffer_pushint(fake * fk, interpreter * inter)
{
	int data = fkpspop<int>(fk);
	buffer * b = fkpspop<buffer *>(fk);
	if (b)
	{
		b->write((const char *)&data, sizeof(data));
		fkpspush<int>(fk, b->size());
	}
	else
	{
		fkpspush<int>(fk, 0);
	}
}
void buildin_buffer_pushchar(fake * fk, interpreter * inter)
{
	char data = fkpspop<char>(fk);
	buffer * b = fkpspop<buffer *>(fk);
	if (b)
	{
		b->write((const char *)&data, sizeof(data));
		fkpspush<int>(fk, b->size());
	}
	else
	{
		fkpspush<int>(fk, 0);
	}
}
void buildin_buffer_pushshort(fake * fk, interpreter * inter)
{
	short data = fkpspop<short>(fk);
	buffer * b = fkpspop<buffer *>(fk);
	if (b)
	{
		b->write((const char *)&data, sizeof(data));
		fkpspush<int>(fk, b->size());
	}
	else
	{
		fkpspush<int>(fk, 0);
	}
}
void buildin_buffer_pushint64(fake * fk, interpreter * inter)
{
	int64_t data = fkpspop<int64_t>(fk);
	buffer * b = fkpspop<buffer *>(fk);
	if (b)
	{
		b->write((const char *)&data, sizeof(data));
		fkpspush<int>(fk, b->size());
	}
	else
	{
		fkpspush<int>(fk, 0);
	}
}
void buildin_buffer_pushstring(fake * fk, interpreter * inter)
{
	int len = fkpspop<int>(fk);
	const char * data = fkpspopcstrptr(fk);
	int srclen = strlen(data);
	buffer * b = fkpspop<buffer *>(fk);
	if (b)
	{
		if (srclen >= len)
		{
			b->write(data, len);
		}
		else
		{
			if (b->can_write(len))
			{
				b->write(data, srclen);
				const char end = 0;
				b->write(&end, sizeof(end));
				b->skip_write(len - srclen - sizeof(end));
			}
		}
		fkpspush<int>(fk, b->size());
	}
	else
	{
		fkpspush<int>(fk, 0);
	}
}
void buildin_buffer_pushbuffer(fake * fk, interpreter * inter)
{
	buffer * ob = fkpspop<buffer *>(fk);
	buffer * b = fkpspop<buffer *>(fk);
	if (b && ob)
	{
		if (b->can_write(ob->size()))
		{
			ob->store();
			b->write(ob->get_read_line_buffer(), ob->get_read_line_size());
			ob->skip_read(ob->get_read_line_size());
			b->write(ob->get_read_line_buffer(), ob->get_read_line_size());
			ob->skip_read(ob->get_read_line_size());
			ob->restore();
		}
		fkpspush<int>(fk, b->size());
	}
	else
	{
		fkpspush<int>(fk, 0);
	}
}
void buildin_buffer_popint(fake * fk, interpreter * inter)
{
	int data = 0;
	buffer * b = fkpspop<buffer *>(fk);
	if (b)
	{
		b->read((char *)&data, sizeof(data));
	}
	fkpspush<int>(fk, data);
}
void buildin_buffer_popshort(fake * fk, interpreter * inter)
{
	short data = 0;
	buffer * b = fkpspop<buffer *>(fk);
	if (b)
	{
		b->read((char *)&data, sizeof(data));
	}
	fkpspush<short>(fk, data);
}
void buildin_buffer_popchar(fake * fk, interpreter * inter)
{
	char data = 0;
	buffer * b = fkpspop<buffer *>(fk);
	if (b)
	{
		b->read((char *)&data, sizeof(data));
	}
	fkpspush<char>(fk, data);
}
void buildin_buffer_popint64(fake * fk, interpreter * inter)
{
	int64_t data = 0;
	buffer * b = fkpspop<buffer *>(fk);
	if (b)
	{
		b->read((char *)&data, sizeof(data));
	}
	fkpspush<int64_t>(fk, data);
}
void buildin_buffer_popstring(fake * fk, interpreter * inter)
{
	int len = fkpspop<int>(fk);
	char * data = (char *)safe_fkmalloc(fk, len + 1);
	memset(data, 0, len + 1);
	buffer * b = fkpspop<buffer *>(fk);
	if (b)
	{
		b->read(data, len);
	}
	fkpspush<const char *>(fk, data);
	safe_fkfree(fk, data);
}
void buildin_buffer_popbuffer(fake * fk, interpreter * inter)
{
	int len = fkpspop<int>(fk);
	buffer * ob = fk->bif.newbuffer(len);
	buffer * b = fkpspop<buffer *>(fk);
	if (b)
	{
		if (b->can_read(len))
		{
			b->read(ob->get_write_line_buffer(), ob->get_write_line_size());
			ob->skip_write(ob->get_write_line_size());
		}
	}
	fkpspush<buffer *>(fk, ob);
}
void buildin_selector_new(fake * fk, interpreter * inter)
{
	int s = fkpspop<int>(fk);
	selector * sel = fk->bif.newselector();
	sel->s = s;
	fkpspush<selector *>(fk, sel);
}
void buildin_selector_set(fake * fk, interpreter * inter)
{
	int s = fkpspop<int>(fk);
	selector * sel = fkpspop<selector *>(fk);
	sel->s = s;
	fkpspush<bool>(fk, true);
}
void buildin_selector_tick(fake * fk, interpreter * inter)
{
	bool isin = false;
	bool isout = false;
	bool iserr = false;
	
	selector * sel = fkpspop<selector *>(fk);
	int eventnum = 0;
	if (sel && sel->s != -1)
	{
		int s = sel->s % FD_SETSIZE;
		
		fd_set readfds;
		fd_set writefds;
		fd_set exceptfds;
		int maxfd;
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		FD_ZERO(&exceptfds);
		maxfd = -1;
		FD_SET(s, &readfds);
		FD_SET(s, &writefds);
		FD_SET(s, &exceptfds);
		maxfd = s + 1;
		
		timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		
		int ret = ::select(maxfd + 1, 
			&readfds,
			&writefds,
			&exceptfds,
			&timeout);
		
		if (ret > 0)
		{
			eventnum = 1;
			if (FD_ISSET(s, &exceptfds) != 0)
			{
				iserr = true;
			}
			if (FD_ISSET(s, &readfds) != 0)
			{
				isin = true;
			}
			if (FD_ISSET(s, &writefds) != 0)
			{
				isout = true;
			}
		}
	}
	fkpspush<int>(fk, eventnum);
	fkpspush<bool>(fk, iserr);
	fkpspush<bool>(fk, isin);
	fkpspush<bool>(fk, isout);
}
void buildin_tcp_socket(fake * fk, interpreter * inter)
{
	int s = ::socket(AF_INET, SOCK_STREAM, 0);
	fkpspush<int>(fk, s);
}
void buildin_tcp_close(fake * fk, interpreter * inter)
{
	int s = fkpspop<int>(fk);
#if defined(WIN32)
	::closesocket(s);
#else
	::close(s);
#endif
	fkpspush<int>(fk, 1);
}
void buildin_tcp_connect(fake * fk, interpreter * inter)
{
	int port = fkpspop<int>(fk);
	const char * ip = fkpspopcstrptr(fk);
	int s = fkpspop<int>(fk);
	
	sockaddr_in _sockaddr;
	memset(&_sockaddr, 0, sizeof(_sockaddr));
	_sockaddr.sin_family = AF_INET;
	_sockaddr.sin_port = htons(port);
	_sockaddr.sin_addr.s_addr = inet_addr(ip);
	
	int ret = ::connect(s, (const sockaddr *)&_sockaddr, sizeof(_sockaddr));
	if (ret != 0)
	{
		fkpspush<bool>(fk, false);
	}
	else
	{
		fkpspush<bool>(fk, true);
	}
}
void buildin_tcp_recv(fake * fk, interpreter * inter)
{
	buffer * b = fkpspop<buffer *>(fk);
	int s = fkpspop<int>(fk);
	if (!b)
	{
		fkpspush<bool>(fk, false);
		fkpspush<int>(fk, 0);
		return;
	}
	
	if (b->full())
	{
		fkpspush<bool>(fk, true);
		fkpspush<int>(fk, 0);
		return;
	}
	
	int len = ::recv(s, b->get_write_line_buffer(), b->get_write_line_size(), 0);
		
	if (len == 0 || len < 0)
	{
		if (GET_NET_ERROR != NET_BLOCK_ERROR && GET_NET_ERROR != NET_BLOCK_ERROR_EX)
		{
			fkpspush<bool>(fk, false);
			fkpspush<int>(fk, 0);
			return;
		}
		if (len < 0)
		{
			len = 0;
		}
	}
	else
	{
		b->skip_write(len);
	}

	fkpspush<bool>(fk, true);
	fkpspush<int>(fk, len);
}
void buildin_tcp_send(fake * fk, interpreter * inter)
{
	buffer * b = fkpspop<buffer *>(fk);
	int s = fkpspop<int>(fk);
	if (!b)
	{
		fkpspush<bool>(fk, false);
		fkpspush<int>(fk, 0);
		return;
	}
	
	if (b->empty())
	{
		fkpspush<bool>(fk, true);
		fkpspush<int>(fk, 0);
		return;
	}
	
	int len = ::send(s, b->get_read_line_buffer(), b->get_read_line_size(), 0);
		
	if (len == 0 || len < 0)
	{
		if (GET_NET_ERROR != NET_BLOCK_ERROR && GET_NET_ERROR != NET_BLOCK_ERROR_EX)
		{
			fkpspush<bool>(fk, false);
			fkpspush<int>(fk, 0);
			return;
		}
		if (len < 0)
		{
			len = 0;
		}
	}
	else
	{
		b->skip_read(len);
	}

	fkpspush<bool>(fk, true);
	fkpspush<int>(fk, len);
}
void buildin_tcp_socket_set_sendbuffer(fake * fk, interpreter * inter)
{
	int size = fkpspop<int>(fk);
	int s = fkpspop<int>(fk);
	::setsockopt(s, SOL_SOCKET, SO_RCVBUF, (const char *)&size, sizeof(int));
	fkpspush<int>(fk, 1);
}
void buildin_tcp_socket_set_recvbuffer(fake * fk, interpreter * inter)
{
	int size = fkpspop<int>(fk);
	int s = fkpspop<int>(fk);
	::setsockopt(s, SOL_SOCKET, SO_SNDBUF, (const char *)&size, sizeof(int));
	fkpspush<int>(fk, 1);
}
void buildin_tcp_socket_set_nonblocking(fake * fk, interpreter * inter)
{
	bool on = fkpspop<bool>(fk);
	int s = fkpspop<int>(fk);
	
#if defined(WIN32)
	ioctlsocket(s, FIONBIO, (u_long *)&on);
#else
	int32_t opts;
	opts = fcntl(s, F_GETFL, 0);

	if (opts >= 0)
	{
		if (on)
		{
			opts = (opts | O_NONBLOCK);
		}
		else
		{
			opts = (opts & ~O_NONBLOCK);
		}

		fcntl(s, F_SETFL, opts);
	}
#endif

	fkpspush<int>(fk, 1);
}
void buildin_tcp_socket_set_linger(fake * fk, interpreter * inter)
{
	int lingertime = fkpspop<int>(fk);
	int s = fkpspop<int>(fk);
	
	linger so_linger;
	so_linger.l_onoff = true;
	so_linger.l_linger = lingertime;
	::setsockopt(s, SOL_SOCKET, SO_LINGER, (const char *)&so_linger, sizeof(so_linger));
	
	fkpspush<int>(fk, 1);
}
void buildin_tcp_socket_get_local_ip_port(fake * fk, interpreter * inter)
{
	int s = fkpspop<int>(fk);
	
	char ip[64] = {0};
	int port = 0;
	
	sockaddr_in _local_sockaddr;
	memset(&_local_sockaddr, 0, sizeof(_local_sockaddr));
	socklen_t size = sizeof(_local_sockaddr);
	if (::getsockname(s, (sockaddr *)&_local_sockaddr, &size) == 0)
	{
		strcpy(ip, inet_ntoa(_local_sockaddr.sin_addr));
		port = htons(_local_sockaddr.sin_port);
	}
	
	fkpspush<const char *>(fk, ip);
	fkpspush<int>(fk, port);
}
void buildin_tcp_bind(fake * fk, interpreter * inter)
{
	int port = fkpspop<int>(fk);
	const char * ip = fkpspopcstrptr(fk);
	int s = fkpspop<int>(fk);
	
	sockaddr_in _sockaddr;
	memset(&_sockaddr, 0, sizeof(_sockaddr));
	_sockaddr.sin_family = AF_INET;
	_sockaddr.sin_port = htons(port);
	_sockaddr.sin_addr.s_addr = inet_addr(ip);
	int ret = ::bind(s, (const sockaddr *)&_sockaddr, sizeof(_sockaddr));
	if (ret != 0)
	{
		fkpspush<bool>(fk, false);
	}
	else
	{
		fkpspush<bool>(fk, true);
	}
}
void buildin_tcp_listen(fake * fk, interpreter * inter)
{
	int backlog = fkpspop<int>(fk);
	int s = fkpspop<int>(fk);
	
	int ret = ::listen(s, backlog);
	if (ret != 0)
	{
		fkpspush<bool>(fk, false);
	}
	else
	{
		fkpspush<bool>(fk, true);
	}
}
void buildin_tcp_accept(fake * fk, interpreter * inter)
{
	int s = fkpspop<int>(fk);
	
	sockaddr_in _sockaddr;
	memset(&_sockaddr, 0, sizeof(_sockaddr));
	socklen_t size = sizeof(_sockaddr);
	int ns = ::accept(s, (sockaddr*)&_sockaddr, &size);
	if (ns == -1)
	{
		fkpspush<bool>(fk, false);
		fkpspush<int>(fk, ns);
	}
	else
	{
		fkpspush<bool>(fk, true);
		fkpspush<int>(fk, ns);
	}
}

void buildinfuncnet::opennetfunc()
{
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("buffer"), buildin_buffer_new);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("buffer_clear"), buildin_buffer_clear);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("buffer_maxsize"), buildin_buffer_maxsize);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("buffer_size"), buildin_buffer_size);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("buffer_store"), buildin_buffer_store);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("buffer_restore"), buildin_buffer_restore);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("buffer_push_int"), buildin_buffer_pushint);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("buffer_push_short"), buildin_buffer_pushshort);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("buffer_push_char"), buildin_buffer_pushchar);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("buffer_push_int64"), buildin_buffer_pushint64);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("buffer_push_string"), buildin_buffer_pushstring);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("buffer_push_buffer"), buildin_buffer_pushbuffer);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("buffer_pop_int"), buildin_buffer_popint);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("buffer_pop_short"), buildin_buffer_popshort);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("buffer_pop_char"), buildin_buffer_popchar);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("buffer_pop_int64"), buildin_buffer_popint64);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("buffer_pop_string"), buildin_buffer_popstring);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("buffer_pop_buffer"), buildin_buffer_popbuffer);
	
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("selector"), buildin_selector_new);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("selector_set"), buildin_selector_set);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("selector_tick"), buildin_selector_tick);

	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("socket"), buildin_tcp_socket);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("close"), buildin_tcp_close);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("connect"), buildin_tcp_connect);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("tcprecv"), buildin_tcp_recv);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("tcpsend"), buildin_tcp_send);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("socket_set_sendbuffer"), buildin_tcp_socket_set_sendbuffer);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("socket_set_recvbuffer"), buildin_tcp_socket_set_recvbuffer);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("socket_set_nonblocking"), buildin_tcp_socket_set_nonblocking);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("socket_set_linger"), buildin_tcp_socket_set_linger);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("socket_get_local_ip_port"), buildin_tcp_socket_get_local_ip_port);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("bind"), buildin_tcp_bind);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("listen"), buildin_tcp_listen);
	m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("accept"), buildin_tcp_accept);
}

buildinfuncnet::buildinfuncnet(fake * fk) : m_fk(fk)
{
	POOLLIST_INI(m_buffer, fk);
	POOLLIST_INI(m_selector, fk);
	// socket
#ifdef WIN32
	WORD wVersionRequested;
	WSADATA wsaData;
	int32_t err;
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
#endif
}

buildinfuncnet::~buildinfuncnet()
{
	clear();
	POOLLIST_DELETE(m_buffer);
	POOLLIST_DELETE(m_selector);
}

void buildinfuncnet::clear()
{
	// 节省内存同时让maxsize严格
	POOLLIST_CLEAR(m_buffer, buffer, BUFFER_DELETE(n->t));
	POOLLIST_CLEAR(m_selector, selector, USE(n->t));
}

buffer * buildinfuncnet::newbuffer(int size)
{
	pool<buffer>::node * n = 0;
	POOLLIST_POP(m_buffer, n, buffer, m_fk->cfg.array_grow_speed);

	n->t.ini(m_fk, size);

	return &n->t;
}

selector * buildinfuncnet::newselector()
{
	pool<selector>::node * n = 0;
	POOLLIST_POP(m_selector, n, selector, m_fk->cfg.array_grow_speed);
	n->t.s = 0;
	return &n->t;
}

