#pragma once

#include "types.h"

#define BUFFER_STORE_SIZE 2

/*
type:		   [1]
iter:	 begin(2)	 end(8)
			|		   |
data:   _ _ * * * * * * _ _ _ 
buffer: _ _ _ _ _ _ _ _ _ _ _ 
index:  0 1 2 3 4 5 6 7 8 9 10

type:		   [2]
iter:	  end(2)   begin(7)
			|		 |
data:   * * _ _ _ _ _ * * * *  
buffer: _ _ _ _ _ _ _ _ _ _ _ 
index:  0 1 2 3 4 5 6 7 8 9 10


type:		   [3]
iter:	  begin(4),end(4)
				|
data:   _ _ _ _ _ _ _ _ _ _ _ 
buffer: _ _ _ _ _ _ _ _ _ _ _ 
index:  0 1 2 3 4 5 6 7 8 9 10

type:		   [4]
iter:	  begin(4),end(4)
|				 |
data:   * * * * * * * * * * *
buffer: _ _ _ _ _ _ _ _ _ _ _
index:  0 1 2 3 4 5 6 7 8 9 10

*/
struct fake;

struct buffer {
    force_inline void ini(fake *fk, size_t len) {
        ini(fk, (char *) safe_fkmalloc(fk, len, emt_buffer), len);
        m_extra = false;
    }

    force_inline void ini(fake *fk, char *buffer, size_t len) {
        m_fk = fk;
        m_size = len;
        m_buffer = buffer;
        m_datasize = 0;
        m_begin = 0;
        m_end = 0;
        m_extra = true;
    }

    force_inline void fini() {
        m_size = 0;
        if (!m_extra) {
            safe_fkfree(m_fk, m_buffer);
        }
        m_buffer = 0;
        m_datasize = 0;
        m_begin = 0;
        m_end = 0;
    }

    force_inline bool can_write(size_t size) {
        return m_datasize + size <= m_size;
    }

    force_inline void skip_write(size_t size) {
        if (UNLIKE(!(m_datasize + size <= m_size))) {
            return;
        }

        m_datasize += size;
        m_end += size;
        if (UNLIKE(m_end >= m_size)) {
            m_end -= m_size;
        }
    }

    force_inline bool write(const char *p, size_t size) {
        if (UNLIKE(!(m_datasize + size <= m_size))) {
            return false;
        }

        if (m_end >= m_begin)    // [1][3]
        {
            // 能装下
            if (m_size - m_end >= size) {
                memcpy(m_buffer + m_end, p, size * sizeof(char));
            } else {
                memcpy(m_buffer + m_end, p, (m_size - m_end) * sizeof(char));
                memcpy(m_buffer, p + (m_size - m_end), (size - (m_size - m_end)) * sizeof(char));
            }
        } else    //[2]
        {
            memcpy(m_buffer + m_end, p, size * sizeof(char));
        }

        m_datasize += size;
        m_end += size;
        if (UNLIKE(m_end >= m_size)) {
            m_end -= m_size;
        }

        return true;
    }

    force_inline bool can_read(size_t size) {
        return m_datasize >= size;
    }

    force_inline void skip_read(size_t size) {
        if (UNLIKE(!(m_datasize >= size))) {
            return;
        }

        m_datasize -= size;
        m_begin += size;
        if (UNLIKE(m_begin >= m_size)) {
            m_begin -= m_size;
        }
        if (!m_datasize) {
            m_begin = 0;
            m_end = 0;
        }
    }

    force_inline bool read(char *out, size_t size) {
        if (UNLIKE(!(m_datasize >= size))) {
            return false;
        }

        if (m_begin >= m_end)    // [2][4]
        {
            // 能读完
            if (m_size - m_begin >= size) {
                memcpy(out, m_buffer + m_begin, size * sizeof(char));
            } else {
                memcpy(out, m_buffer + m_begin, (m_size - m_begin) * sizeof(char));
                memcpy(out + (m_size - m_begin), m_buffer, (size - (m_size - m_begin)) * sizeof(char));
            }
        } else    // [1]
        {
            memcpy(out, m_buffer + m_begin, size * sizeof(char));
        }

        m_datasize -= size;
        m_begin += size;
        if (UNLIKE(m_begin >= m_size)) {
            m_begin -= m_size;
        }

        return true;
    }

    force_inline void store(int i) {
        assert(i >= 0 && i < BUFFER_STORE_SIZE);
        if (LIKE(i >= 0 && i < BUFFER_STORE_SIZE)) {
            m_store_datasize[i] = m_datasize;
            m_store_begin[i] = m_begin;
            m_store_end[i] = m_end;
        }
    }

    force_inline void restore(int i) {
        assert(i >= 0 && i < BUFFER_STORE_SIZE);
        if (LIKE(i >= 0 && i < BUFFER_STORE_SIZE)) {
            m_datasize = m_store_datasize[i];
            m_begin = m_store_begin[i];
            m_end = m_store_end[i];
        }
    }

    force_inline void clear() {
        m_datasize = 0;
        m_begin = 0;
        m_end = 0;
    }

    force_inline size_t size() const {
        return m_datasize;
    }

    force_inline size_t capacity() const {
        return m_size;
    }

    force_inline bool empty() const {
        return size() == 0;
    }

    force_inline bool full() const {
        return size() == capacity();
    }

    force_inline const char *get_read_line_buffer() const {
        return m_buffer + m_begin;
    }

    force_inline size_t get_read_line_size() const {
        return FKMIN(m_datasize, m_size - m_begin);
    }

    force_inline char *get_write_line_buffer() const {
        return m_buffer + m_end;
    }

    force_inline size_t get_write_line_size() const {
        return FKMIN(m_size - m_datasize, m_size - m_end);
    }

    force_inline char *get_buffer() const {
        return m_buffer;
    }

private:
    fake *m_fk;
    char *m_buffer;
    size_t m_size;
    size_t m_datasize;
    size_t m_begin;
    size_t m_end;
    size_t m_store_datasize[BUFFER_STORE_SIZE];
    size_t m_store_begin[BUFFER_STORE_SIZE];
    size_t m_store_end[BUFFER_STORE_SIZE];
    bool m_extra;
};

#define BUFFER_DELETE(b) (b).fini()

