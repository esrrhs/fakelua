#pragma once

#include "types.h"

struct selector
{
	fd_set m_readfds;
	fd_set m_writefds;
	fd_set m_exceptfds;
	fd_set m_tmpreadfds;
	fd_set m_tmpwritefds;
	fd_set m_tmpexceptfds;
	int m_maxfd;
};

#define SELECTOR_CLR(sel) \
    { \
    	FD_ZERO(&(sel).m_readfds); \
    	FD_ZERO(&(sel).m_writefds); \
    	FD_ZERO(&(sel).m_exceptfds); \
    	FD_ZERO(&(sel).m_tmpreadfds); \
    	FD_ZERO(&(sel).m_tmpwritefds); \
    	FD_ZERO(&(sel).m_tmpexceptfds); \
    	(sel).m_maxfd = 0; \
    }
    
#define SELECTOR_TICK(sel, n) \
    { \
    	timeval timeout; \
    	timeout.tv_sec = 0; \
    	timeout.tv_usec = 0; \
    	(sel).m_tmpreadfds = (sel).m_readfds; \
    	(sel).m_tmpwritefds = (sel).m_writefds; \
    	(sel).m_tmpexceptfds = (sel).m_exceptfds; \
    	n = ::select((sel).m_maxfd + 1,  \
    		&(sel).m_tmpreadfds, \
    		&(sel).m_tmpwritefds, \
    		&(sel).m_tmpexceptfds, \
    		&timeout); \
    }

#define SELECTOR_ADD(sel, s) \
    { \
		FD_SET(s, &(sel).m_readfds); \
		FD_SET(s, &(sel).m_writefds); \
		FD_SET(s, &(sel).m_exceptfds); \
		(sel).m_maxfd = FKMAX((sel).m_maxfd, s + 1); \
    }

#define SELECTOR_DEL(sel, s) \
    { \
		FD_CLR(s, &(sel).m_readfds); \
		FD_CLR(s, &(sel).m_writefds); \
		FD_CLR(s, &(sel).m_exceptfds); \
    }

#define SELECTOR_IN(sel, s, b) \
    { \
		b = FD_ISSET(s, &(sel).m_tmpreadfds); \
    }

#define SELECTOR_OUT(sel, s, b) \
    { \
		b = FD_ISSET(s, &(sel).m_tmpwritefds); \
    }

#define SELECTOR_ERR(sel, s, b) \
    { \
		b = FD_ISSET(s, &(sel).m_tmpexceptfds); \
    }

