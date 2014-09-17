#pragma once

#include "types.h"
#include "fuck.h"

class variant 
{
public:

	enum Type 
	{
		NIL,		
		
		// atomic types 		
		BOOL,
		INT,
		REAL,
		STRING,

		// pointer
		POINTER,
		
		VARIANT_MAX
	};	
    
	force_inline variant(const variant& p_variant) : m_fk(p_variant.m_fk)
    {
    	m_type = NIL;
    	reference(p_variant);
    }
	force_inline variant() { m_type = NIL; m_fk = 0; }
	force_inline variant(fuck * fk) { m_type = NIL; m_fk = fk; }
	force_inline ~variant() { if (m_type != variant::NIL) clear(); }

	force_inline variant(bool p_bool, fuck * fk) : m_fk(fk)
    {
    	m_type = BOOL;
    	m_data.m_bool = p_bool;
    }
	force_inline variant(signed int p_int, fuck * fk) : m_fk(fk)
    {
    	m_type = INT;
    	m_data.m_int = p_int;
    }
	force_inline variant(unsigned int p_int, fuck * fk) : m_fk(fk)
    {
    	m_type = INT;
    	m_data.m_int = p_int;
    }

	force_inline variant(signed short p_short, fuck * fk) : m_fk(fk)
    {
    	m_type = INT;
    	m_data.m_int = p_short;
    }
	force_inline variant(unsigned short p_short, fuck * fk) : m_fk(fk)
    {
    	m_type = INT;
    	m_data.m_int = p_short;
    }
	force_inline variant(signed char p_char, fuck * fk) : m_fk(fk)
    {
    	m_type = INT;
    	m_data.m_int = p_char;
    }
	force_inline variant(unsigned char p_char, fuck * fk) : m_fk(fk)
    {
    	m_type = INT;
    	m_data.m_int = p_char;
    }
	force_inline variant(int64_t p_int, fuck * fk) : m_fk(fk)
    {
    	m_type = INT;
    	m_data.m_int = p_int;
    }
	force_inline variant(uint64_t p_int, fuck * fk) : m_fk(fk)
    {
    	m_type = INT;
    	m_data.m_int = p_int;
    }

	force_inline variant(float p_float, fuck * fk) : m_fk(fk)
    {
    	m_type = REAL;
    	m_data.m_real = p_float;
    }
	force_inline variant(double p_double, fuck * fk) : m_fk(fk)
    {
    	m_type = REAL;
    	m_data.m_real = p_double;
    }
	force_inline variant(const String& p_string, fuck * fk) : m_fk(fk)
    {
    	m_type = STRING;
    	m_data.m_str = fknew<String>(fk, p_string);
    }

	force_inline variant(const char * const p_cstring, fuck * fk) : m_fk(fk)
    {
    	m_type = STRING;
    	m_data.m_str = fknew<String>(fk, p_cstring);
    }
	force_inline variant(void * p_ptr, fuck * fk) : m_fk(fk)
    {
    	m_type = POINTER;
    	m_data.m_ptr = p_ptr;
    }
	
	force_inline bool operator==(const variant& p_variant) const
    {
    	if (m_type != p_variant.m_type) //evaluation of operator== needs to be more strict
    		return false;
    		
    	switch( m_type ) 
    	{
    		case NIL: 
    			return true;

    	    // atomic types
    		case BOOL: 
    			return m_data.m_bool == p_variant.m_data.m_bool;
    		
    		case INT: 
    			return m_data.m_int == p_variant.m_data.m_int;
    		
    		case REAL: 
    			return m_data.m_real == p_variant.m_data.m_real;
    		
    		case STRING: 
    		{
    		    if (m_data.m_str && p_variant.m_data.m_str)
    		    {
    		        return *m_data.m_str == *p_variant.m_data.m_str;
    		    }
    			return m_data.m_str == p_variant.m_data.m_str;
    		} 
    		break;

    		case POINTER: 
    			return m_data.m_ptr == p_variant.m_data.m_ptr;

    		default: 
    			return false;
    	}

    	return false;
    }
	
	force_inline void operator=(const variant& p_variant)
    {
    	clear();
    	reference(p_variant);
    }

    force_inline void plus(const variant& left, const variant& right)
    {
        if (left.m_type == REAL || right.m_type == REAL)
        {
            m_data.m_real = (double)left + (double)right;
            m_type = REAL;
        }
        else
        {
            m_data.m_int = (int64_t)left + (int64_t)right;
            m_type = INT;
        }
    }

    force_inline void minus(const variant& left, const variant& right)
    {
        if (left.m_type == REAL || right.m_type == REAL)
        {
            m_data.m_real = (double)left - (double)right;
            m_type = REAL;
        }
        else
        {
            m_data.m_int = (int64_t)left - (int64_t)right;
            m_type = INT;
        }
    }

    force_inline void multiply(const variant& left, const variant& right)
    {
        if (left.m_type == REAL || right.m_type == REAL)
        {
            m_data.m_real = (double)left * (double)right;
            m_type = REAL;
        }
        else
        {
            m_data.m_int = (int64_t)left * (int64_t)right;
            m_type = INT;
        }
    }

    force_inline void divide(const variant& left, const variant& right)
    {
        if (left.m_type == REAL || right.m_type == REAL)
        {
            m_data.m_real = (double)left / (double)right;
            m_type = REAL;
        }
        else
        {
            m_data.m_int = (int64_t)left / (int64_t)right;
            m_type = INT;
        }
    }

    force_inline void divide_mode(const variant& left, const variant& right)
    {
        m_data.m_int = (int64_t)left % (int64_t)right;
        m_type = INT;
    }

    force_inline void band(const variant& left, const variant& right)
    {
        m_data.m_bool = (bool)left & (bool)right;
        m_type = BOOL;
    }

    force_inline void bor(const variant& left, const variant& right)
    {
        m_data.m_bool = (bool)left | (bool)right;
        m_type = BOOL;
    }

    force_inline void less(const variant& left, const variant& right)
    {
        if (left.m_type == REAL || right.m_type == REAL)
        {
            m_data.m_bool = (double)left < (double)right;
            m_type = BOOL;
        }
        else
        {
            m_data.m_bool = (int64_t)left < (int64_t)right;
            m_type = BOOL;
        }
    }

    force_inline void more(const variant& left, const variant& right)
    {
        if (left.m_type == REAL || right.m_type == REAL)
        {
            m_data.m_bool = (double)left > (double)right;
            m_type = BOOL;
        }
        else
        {
            m_data.m_bool = (int64_t)left > (int64_t)right;
            m_type = BOOL;
        }
    }

    force_inline void equal(const variant& left, const variant& right)
    {
        if (left.m_type == REAL || right.m_type == REAL)
        {
            m_data.m_bool = (double)left == (double)right;
            m_type = BOOL;
        }
        else
        {
            m_data.m_bool = (int64_t)left == (int64_t)right;
            m_type = BOOL;
        }
    }

    force_inline void moreequal(const variant& left, const variant& right)
    {
        if (left.m_type == REAL || right.m_type == REAL)
        {
            m_data.m_bool = (double)left >= (double)right;
            m_type = BOOL;
        }
        else
        {
            m_data.m_bool = (int64_t)left >= (int64_t)right;
            m_type = BOOL;
        }
    }

    force_inline void lessequal(const variant& left, const variant& right)
    {
        if (left.m_type == REAL || right.m_type == REAL)
        {
            m_data.m_bool = (double)left <= (double)right;
            m_type = BOOL;
        }
        else
        {
            m_data.m_bool = (int64_t)left <= (int64_t)right;
            m_type = BOOL;
        }
    }

    force_inline void notequal(const variant& left, const variant& right)
    {
        if (left.m_type == REAL || right.m_type == REAL)
        {
            m_data.m_bool = (double)left != (double)right;
            m_type = BOOL;
        }
        else
        {
            m_data.m_bool = (int64_t)left != (int64_t)right;
            m_type = BOOL;
        }
    }

	
	force_inline operator bool() const
    {
    	switch(m_type) 
    	{
    		case NIL: return false;
    		case BOOL: return m_data.m_bool;
    		case INT: return m_data.m_int;
    		case REAL: return m_data.m_real;
    		case STRING: return !((reinterpret_cast<const String*>(m_data.m_str))->empty());
    		case POINTER: return m_data.m_ptr;
    		default: 
    	        return false;
    	}

    	return false;
    }
    
	force_inline operator int() const
    {
    	switch( m_type ) 
    	{
    		case BOOL: return m_data.m_bool ? 1 : 0;
    		case INT: return m_data.m_int;
    		case REAL: return m_data.m_real;
    		case STRING: return fkatoi(m_data.m_str);
    		default: 
    		    return 0;
    	}
    	
    	return 0;
    }

	force_inline operator unsigned int() const
    {
    	switch( m_type ) 
    	{
    		case BOOL: return m_data.m_bool ? 1 : 0;
    		case INT: return m_data.m_int;
    		case REAL: return m_data.m_real;
    		case STRING: return fkatoi(m_data.m_str);
    		default: 
    		    return 0;
    	}
    	
    	return 0;
    }
    
	force_inline operator short() const
    {
    	switch( m_type ) 
    	{
    		case BOOL: return m_data.m_bool ? 1 : 0;
    		case INT: return m_data.m_int;
    		case REAL: return m_data.m_real;
    		case STRING: return fkatoi(m_data.m_str);
    		default: 
    		    return 0;
    	}
    	
    	return 0;
    }
    
	force_inline operator unsigned short() const 
    {
    	switch( m_type ) 
    	{
    		case BOOL: return m_data.m_bool ? 1 : 0;
    		case INT: return m_data.m_int;
    		case REAL: return m_data.m_real;
    		case STRING: return fkatoi(m_data.m_str);
    		default: 
    		    return 0;
    	}
    	
    	return 0;
    }
    
	force_inline operator char() const
    {
    	switch( m_type ) 
    	{
    		case BOOL: return m_data.m_bool ? 1 : 0;
    		case INT: return m_data.m_int;
    		case REAL: return m_data.m_real;
    		case STRING: return fkatoi(m_data.m_str);
    		default: 
    		    return 0;
    	}
    	
    	return 0;
    }
	force_inline operator unsigned char() const
    {
    	switch( m_type ) 
    	{
    		case BOOL: return m_data.m_bool ? 1 : 0;
    		case INT: return m_data.m_int;
    		case REAL: return m_data.m_real;
    		case STRING: return fkatoi(m_data.m_str);
    		default: 
    		    return 0;
    	}
    	
    	return 0;
    }

	force_inline operator int64_t() const
    {
    	switch( m_type ) 
    	{
    		case BOOL: return m_data.m_bool ? 1 : 0;
    		case INT: return m_data.m_int;
    		case REAL: return m_data.m_real;
    		case STRING: return fkatol(m_data.m_str);
    		default: 
    		    return 0;
    	}
    	return 0;
    }
    
	force_inline operator uint64_t() const
	{

    	switch( m_type ) 
    	{
    		case BOOL: return m_data.m_bool ? 1 : 0;
    		case INT: return m_data.m_int;
    		case REAL: return m_data.m_real;
    		case STRING: return fkatol(m_data.m_str);
    		default: 
    		    return 0;
    	}

    	return 0;
    }
	force_inline operator float() const
    {
    	switch( m_type ) 
    	{
    		case BOOL: return m_data.m_bool ? 1.0 : 0.0;
    		case INT: return (float)m_data.m_int;
    		case REAL: return m_data.m_real;
    		case STRING: return fkatof(m_data.m_str);
    		default: 
    		    return 0;
    	}
    	
    	return 0;
    }
	force_inline operator double() const
    {
    	switch( m_type ) 
    	{
    		case BOOL: return m_data.m_bool ? 1.0 : 0.0;
    		case INT: return (float)m_data.m_int;
    		case REAL: return m_data.m_real;
    		case STRING: return fkatod(m_data.m_str);
    		default: 
    		    return 0;
    	}
    	
    	return 0;
    }
	force_inline operator String() const
    {
    	switch( m_type ) 
    	{
    		case BOOL: return m_data.m_bool ? "true" : "false";
    		case INT: return fkitoa(m_data.m_int);
    		case REAL: return fkdtoa(m_data.m_real);
    		case STRING: return *reinterpret_cast<const String*>(m_data.m_str);
    		default: 
    		    return "";
    	}
    	
    	return "";
    }
	force_inline operator void*() const
    {
    	switch( m_type ) 
    	{
    		case POINTER: return m_data.m_ptr; 
    		default: 
    		    return 0;
    	}
    	
    	return 0;
    }

	force_inline Type get_type() const { return m_type; }
	static force_inline String get_type_name(variant::Type p_type)
    {
    	switch( p_type ) 
    	{
    		case NIL: 
    			return "Nil";

    	    // atomic types
    		case BOOL: 
    			return "bool";
    		
    		case INT: 
    			return "int";
    		
    		case REAL: 
    			return "float";
    		
    		case STRING: 
    			return "string";

    		case POINTER: 
    			return "pointer";

    		default: 
    			return "none";
    	}

    	return "";
    }

	force_inline uint32_t hash() const
    {
    	switch( m_type ) 
    	{
    		case NIL: 
    			return 0;
    		
    		case BOOL: 
    			return m_data.m_bool ? 1 : 0;
    		
    		case INT: 
    			return m_data.m_int;
    		
    		case REAL: 
    		{
    			MarshallFloat mf;
    			mf.f = m_data.m_real;
    			return mf.i;
    		} 
    		break;
    		
    		case STRING: 
    			return fkstrhash(m_data.m_str);
    		
    		case POINTER: 
    		{   
    		    MarshallPoiner mp;
    			mp.p = m_data.m_ptr;
    			return mp.i;
    		} 
    		break;
    		
    		default: 
    	        return 0;

    	}

    	return 0;
    }

private:

	union MemData
	{
		bool m_bool;
		int64_t m_int;
		double m_real;
		String * m_str;
		void * m_ptr; //generic pointer
	};

	force_inline void clear()
    {
    	switch(m_type) 
    	{
    		case STRING: 
    		{
    		    fkdelete(m_fk, m_data.m_str);
    		    m_data.m_str = 0;
    		} 
    		break;
    		
    		default: 
    		    break;
    	}
    	
    	m_type = NIL;
    }
    
	force_inline void reference(const variant& p_variant)
    {
    	if (this == &p_variant)
    		return;
    		
    	clear();
    		
    	m_type = p_variant.m_type;
    		
    	switch( p_variant.m_type ) 
    	{
    		// atomic types 		
    		case BOOL:
    		case INT:
    		case REAL:
    		case POINTER:
    		    memcpy(&m_data, &p_variant.m_data, sizeof(m_data));
    		    return;
    		
    		case STRING: 
    		{
    		    if (m_data.m_str)
    		    {
    		        m_data.m_str->clear();
    		    }
    		    else
    		    {
    		        m_data.m_str = fknew<String>(m_fk);
    		    }
    		    
    		    if (p_variant.m_data.m_str)
    		    {
    		        *m_data.m_str = *p_variant.m_data.m_str;
    		    }
    		    else
    			{
    				m_data.m_str->clear();
    		    }
    		} 
    		break;
    		
    		default:
    		    return;
    	}		
    }
	
private:

	Type m_type;

	MemData m_data;

	fuck * m_fk;
};

typedef std::vector<variant> const_list;
typedef std::vector<variant> variant_list;

