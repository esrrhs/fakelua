#include "variant.h"
#include "fuck.h"

String variant::get_type_name(variant::Type p_type) 
{
	
	switch( p_type ) 
	{
		case NIL: 
		{
			return "Nil";
		} 
		break;

	    // atomic types
		case BOOL: 
		{
			return "bool";
		} 
		break;
		
		case INT: 
		{
			return "int";
		} 
		break;
		
		case REAL: 
		{
			return "float";
		} 
		break;
		
		case STRING: 
		{
			return "String";
		} 
		break;

		case POINTER: 
		{
			return "Pointer";
		} 
		break;

		default: 
		{
			return "None";
		} 
		break;
	}

	return "";
}

bool variant::operator==(const variant& p_variant) const 
{
	if (m_type != p_variant.m_type) //evaluation of operator== needs to be more strict
		return false;
		
	switch( m_type ) 
	{
		case NIL: 
		{
			return true;
		} 
		break;

	    // atomic types
		case BOOL: 
		{
			return m_data.m_bool == p_variant.m_data.m_bool;
		} 
		break;
		
		case INT: 
		{
			return m_data.m_int == p_variant.m_data.m_int;
		} 
		break;
		
		case REAL: 
		{
			return m_data.m_real == p_variant.m_data.m_real;
		} 
		break;
		
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
		{
			return m_data.m_ptr == p_variant.m_data.m_ptr;
		} 
		break;

		default: 
		{
			return false;
		} 
		break;
	}

	return false;
}

void variant::operator=(const variant& p_variant)
{
	clear();
	reference(p_variant);
}

void variant::clear() 
{

	switch(m_type) 
	{
		case NIL: 
		case BOOL: 
		case INT: 
		case REAL: 
		{
			// nothing
		} 
		break;
		
		case STRING: 
		{
		    fkdelete(m_fk, m_data.m_str);
		} 
		break;
		
		case POINTER: 
		{
			// nothing
		} 
		break;
		
		default: 
		{
		} /* not needed */
		break;
	}
	
	m_type = NIL;
	memset(&m_data, 0, sizeof(m_data));

}

void variant::reference(const variant& p_variant) 
{
	if (this == &p_variant)
		return;
		
	clear();
		
	m_type = p_variant.m_type;
		
	switch( p_variant.m_type ) 
	{
		case NIL: 
		{
			// none
		} break;
		
		// atomic types 		
		case BOOL: 
		{		
			m_data.m_bool = p_variant.m_data.m_bool;
		} 
		break;
		
		case INT: 
		{
			m_data.m_int = p_variant.m_data.m_int;
		} 
		break;
		
		case REAL: 
		{
			m_data.m_real = p_variant.m_data.m_real;
		} 
		break;
		
		case STRING: 
		{
		    if (m_data.m_str)
		    {
		        *m_data.m_str = String();
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
		        *m_data.m_str = String();
		    }
		} 
		break;
		
		case POINTER: 
		{
		    // ref
		    m_data.m_ptr = p_variant.m_data.m_ptr;
		} 
		break;
		
		default:
		{
		}
		break;
	}		

}

variant::operator int() const 
{

	switch( m_type ) 
	{
		case NIL: return 0; 
		case BOOL: return m_data.m_bool ? 1 : 0;
		case INT: return m_data.m_int;
		case REAL: return m_data.m_real;
		case STRING: return fkatoi(m_data.m_str);
		case POINTER: return 0;
		default: 
		{
			return 0;
		}		
	}
	
	return 0;
}
variant::operator unsigned int() const 
{

	switch( m_type ) 
	{
		case NIL: return 0; 
		case BOOL: return m_data.m_bool ? 1 : 0;
		case INT: return m_data.m_int;
		case REAL: return m_data.m_real;
		case STRING: return fkatoi(m_data.m_str);
		case POINTER: return 0;
		default: 
		{
			return 0;
		}		
	}
	
	return 0;
}

variant::operator int64_t() const 
{

	switch( m_type ) 
	{
		case NIL: return 0;
		case BOOL: return m_data.m_bool ? 1 : 0;
		case INT: return m_data.m_int;
		case REAL: return m_data.m_real;
		case STRING: return fkatol(m_data.m_str);
		case POINTER: return 0;
		default: 
		{
			return 0;
		}
	}

	return 0;
}


variant::operator uint64_t() const {

	switch( m_type ) 
	{
		case NIL: return 0;
		case BOOL: return m_data.m_bool ? 1 : 0;
		case INT: return m_data.m_int;
		case REAL: return m_data.m_real;
		case STRING: return fkatol(m_data.m_str);
		case POINTER: return 0;
		default: 
		{
			return 0;
		}
	}

	return 0;
}

variant::operator short() const 
{
	switch( m_type ) 
	{
		case NIL: return 0; 
		case BOOL: return m_data.m_bool ? 1 : 0;
		case INT: return m_data.m_int;
		case REAL: return m_data.m_real;
		case STRING: return fkatoi(m_data.m_str);
		case POINTER: return 0;
		default: 
		{
			return 0;
		}		
	}
	
	return 0;
}
variant::operator unsigned short() const 
{
	switch( m_type ) 
	{
		case NIL: return 0; 
		case BOOL: return m_data.m_bool ? 1 : 0;
		case INT: return m_data.m_int;
		case REAL: return m_data.m_real;
		case STRING: return fkatoi(m_data.m_str);
		case POINTER: return 0;
		default: 
		{
			return 0;
		}		
	}
	
	return 0;
}
variant::operator char() const 
{
	switch( m_type ) 
	{
		case NIL: return 0; 
		case BOOL: return m_data.m_bool ? 1 : 0;
		case INT: return m_data.m_int;
		case REAL: return m_data.m_real;
		case STRING: return fkatoi(m_data.m_str);
		case POINTER: return 0;
		default: 
		{
			return 0;
		}		
	}
	
	return 0;
}
variant::operator unsigned char() const 
{
	switch( m_type ) 
	{
		case NIL: return 0; 
		case BOOL: return m_data.m_bool ? 1 : 0;
		case INT: return m_data.m_int;
		case REAL: return m_data.m_real;
		case STRING: return fkatoi(m_data.m_str);
		case POINTER: return 0;
		default: 
		{
			return 0;
		}		
	}
	
	return 0;
}

variant::operator float() const 
{
	switch( m_type ) 
	{
		case NIL: return 0; 
		case BOOL: return m_data.m_bool ? 1.0 : 0.0;
		case INT: return (float)m_data.m_int;
		case REAL: return m_data.m_real;
		case STRING: return fkatof(m_data.m_str);
		case POINTER: return 0;
		default: 
		{
			return 0;
		}		
	}
	
	return 0;
}
variant::operator double() const 
{
	switch( m_type ) 
	{
		case NIL: return 0; 
		case BOOL: return m_data.m_bool ? 1.0 : 0.0;
		case INT: return (float)m_data.m_int;
		case REAL: return m_data.m_real;
		case STRING: return fkatod(m_data.m_str);
		case POINTER: return 0;
		default: 
		{
			return 0;
		}		
	}
	
	return 0;
}

variant::operator String() const 
{
	switch( m_type ) 
	{
		case NIL: return ""; 
		case BOOL: return m_data.m_bool ? "True" : "False";
		case INT: return fkitoa(m_data.m_int);
		case REAL: return fkdtoa(m_data.m_real);
		case STRING: return *reinterpret_cast<const String*>(m_data.m_str);
		case POINTER: return ""; 
		default: 
		{
			return "None";
		}		
	}
	
	return "";
}

variant::operator bool() const
{
    return booleanize();
}

variant::operator void*() const
{
	switch( m_type ) 
	{
		case NIL: return 0; 
		case BOOL: return 0;
		case INT: return 0;
		case REAL: return 0;
		case STRING: return 0;
		case POINTER: return m_data.m_ptr; 
		default: 
		{
			return 0;
		}		
	}
	
	return 0;
}

variant::variant(bool p_bool, fuck * fk) : m_fk(fk)
{
	m_type = BOOL;
	m_data.m_bool = p_bool;
}

variant::variant(signed int p_int, fuck * fk) : m_fk(fk)
{
	m_type = INT;
	m_data.m_int = p_int;
}

variant::variant(unsigned int p_int, fuck * fk) : m_fk(fk)
{
	m_type = INT;
	m_data.m_int = p_int;
}

variant::variant(int64_t p_int, fuck * fk) : m_fk(fk)
{
	m_type = INT;
	m_data.m_int = p_int;
}

variant::variant(uint64_t p_int, fuck * fk) : m_fk(fk)
{
	m_type = INT;
	m_data.m_int = p_int;
}

variant::variant(signed short p_short, fuck * fk) : m_fk(fk)
{
	m_type = INT;
	m_data.m_int = p_short;
}
variant::variant(unsigned short p_short, fuck * fk) : m_fk(fk)
{
	m_type = INT;
	m_data.m_int = p_short;
}
variant::variant(signed char p_char, fuck * fk) : m_fk(fk)
{
	m_type = INT;
	m_data.m_int = p_char;
}
variant::variant(unsigned char p_char, fuck * fk) : m_fk(fk)
{
	m_type = INT;
	m_data.m_int = p_char;
}
variant::variant(float p_float, fuck * fk) : m_fk(fk)
{
	m_type = REAL;
	m_data.m_real = p_float;
}
variant::variant(double p_double, fuck * fk) : m_fk(fk)
{
	m_type = REAL;
	m_data.m_real = p_double;
}

variant::variant(const String& p_string, fuck * fk) : m_fk(fk) 
{
	m_type = STRING;
	m_data.m_str = fknew<String>(fk, p_string);
}

variant::variant(const char * const p_cstring, fuck * fk) : m_fk(fk)
{
	m_type = STRING;
	m_data.m_str = fknew<String>(fk, p_cstring);
}

variant::variant(void * p_ptr, fuck * fk) : m_fk(fk)
{
	m_type = POINTER;
	m_data.m_ptr = p_ptr;
}

variant::variant(const variant& p_variant) : m_fk(p_variant.m_fk)
{
	m_type = NIL;
	reference(p_variant);
}

uint32_t variant::hash() const 
{
	switch( m_type ) 
	{
		case NIL: 
		{
			return 0;
		} 
		break;
		
		case BOOL: 
		{
			return m_data.m_bool ? 1 : 0;
		} 
		break;
		
		case INT: 
		{
			return m_data.m_int;
		} 
		break;
		
		case REAL: 
		{
			MarshallFloat mf;
			mf.f = m_data.m_real;
			return mf.i;
		} 
		break;
		
		case STRING: 
		{
			return fkstrhash(m_data.m_str);
		} 
		break;
		
		case POINTER: 
		{   
		    MarshallPoiner mp;
			mp.p = m_data.m_ptr;
			return mp.i;
		} 
		break;
		
		default: 
		{
		}
		break;

	}

	return 0;
}

bool variant::booleanize() const 
{
	switch(m_type) 
	{
		case NIL: return false;
		case BOOL: return m_data.m_bool;
		case INT: return m_data.m_int;
		case REAL: return m_data.m_real;
		case STRING: return (*reinterpret_cast<const String*>(m_data.m_str)) != "";
		case POINTER: return m_data.m_ptr;
		default: 
		{
		}
		break;
	}

	return false;
}
