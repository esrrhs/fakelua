#include "variant.h"

String Variant::get_type_name(Variant::Type p_type) 
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


bool Variant::can_convert(Variant::Type p_type_from, Variant::Type p_type_to) 
{

	if (p_type_from == p_type_to)
		return true;
		
	if (p_type_to == NIL && p_type_from != NIL) //nil can convert to anything
		return true;

	if (p_type_from == NIL) 
	{
		return (p_type_to == POINTER);
	}

	const Type *valid_types = NULL;
	const Type *invalid_types = NULL;

	switch (p_type_to) 
	{
		case BOOL: 
		{

			static const Type valid[] =
			{
				INT,
				REAL,
				STRING,
				NIL,
			};

			valid_types = valid;
		} 
		break;
		
		case INT: 
		{

			static const Type valid[] =
			{
				BOOL,
				REAL,
				STRING,
				NIL,
			};

			valid_types = valid;

		} 
		break;
		
		case REAL: 
		{

			static const Type valid[]=
			{
				BOOL,
				INT,
				STRING,
				NIL,
			};

			valid_types = valid;

		} 
		break;
		
		case STRING: 
		{


			static const Type invalid[]=
			{
				POINTER,
				NIL,
			};

			invalid_types = invalid;
		} 
		break;
		
		case POINTER: 
		{
		} 
		break;
		
		default: 
		{
		}
		break;
	}

	if (valid_types) 
	{
		int i = 0;
		while(valid_types[i] != NIL) 
		{
			if (p_type_from == valid_types[i])
				return true;
			i++;
		}
	} 
	else if (invalid_types) 
	{
		int i = 0;
		while(invalid_types[i] != NIL) 
		{
			if (p_type_from == invalid_types[i])
				return false;
			i++;
		}
	}

	return false;

}

bool Variant::operator==(const Variant& p_variant) const 
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

bool Variant::is_zero() const 
{

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
			return m_data.m_bool == false;
		} 
		break;
		
		case INT: 
		{
			return m_data.m_int == 0;
		} 
		break;
		
		case REAL: 
		{
			return m_data.m_real == 0;
		} 
		break;
		
		case STRING: 
		{
			return *reinterpret_cast<const String*>(m_data.m_str) == String();
		} 
		break;

		case POINTER: 
		{
			return m_data.m_ptr == 0;
		} 
		break;
		
		default: 
		{
		}
		break;
	}

	return false;
}

void Variant::reference(const Variant& p_variant) 
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
			m_data.m_int=p_variant.m_data.m_int;
		} 
		break;
		
		case REAL: 
		{
			m_data.m_real=p_variant.m_data.m_real;
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
		        m_data.m_str = new String();
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

void Variant::clear() 
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
		    if (m_data.m_str)
		    {
		        delete m_data.m_str;
		    }
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

Variant::operator signed int() const 
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
Variant::operator unsigned int() const 
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

Variant::operator int64_t() const 
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


Variant::operator uint64_t() const {

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

Variant::operator signed short() const 
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
Variant::operator unsigned short() const 
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
Variant::operator signed char() const 
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
Variant::operator unsigned char() const 
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

Variant::operator float() const 
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
Variant::operator double() const 
{
	switch( m_type ) 
	{
		case NIL: return 0; 
		case BOOL: return m_data.m_bool ? 1.0 : 0.0;
		case INT: return (float)m_data.m_int;
		case REAL: return m_data.m_real;
		case STRING: return fkatod(m_data.m_str);
		default: 
		{
			return 0;
		}		
	}
	
	return true;
}

Variant::operator String() const 
{
	switch( m_type ) 
	{
		case NIL: return ""; 
		case BOOL: return m_data.m_bool ? "True" : "False";
		case INT: return fkitoa(m_data.m_int);
		case REAL: return fkdtoa(m_data.m_real);
		case STRING: return *reinterpret_cast<const String*>(m_data.m_str);
		default: 
		{
			return "None";
		}		
	}
	
	return "";
}

Variant::Variant(bool p_bool) 
{
	m_type = BOOL;
	m_data.m_bool = p_bool;
}

Variant::Variant(signed int p_int) 
{
	m_type = INT;
	m_data.m_int = p_int;
}

Variant::Variant(unsigned int p_int) 
{
	m_type = INT;
	m_data.m_int = p_int;
}

Variant::Variant(int64_t p_int) 
{
	m_type = INT;
	m_data.m_int = p_int;
}

Variant::Variant(uint64_t p_int) 
{
	m_type = INT;
	m_data.m_int = p_int;
}

Variant::Variant(signed short p_short) 
{
	m_type = INT;
	m_data.m_int = p_short;
}
Variant::Variant(unsigned short p_short) 
{
	m_type = INT;
	m_data.m_int = p_short;
}
Variant::Variant(signed char p_char) 
{
	m_type = INT;
	m_data.m_int = p_char;
}
Variant::Variant(unsigned char p_char) 
{
	m_type = INT;
	m_data.m_int = p_char;
}
Variant::Variant(float p_float) 
{
	m_type = REAL;
	m_data.m_real = p_float;
}
Variant::Variant(double p_double) 
{
	m_type = REAL;
	m_data.m_real = p_double;
}

Variant::Variant(const String& p_string) {

	m_type = STRING;
	m_data.m_str = new String(p_string);
}

Variant::Variant(const char * const p_cstring) 
{
	m_type = STRING;
	m_data.m_str = new String(p_cstring);
}

Variant::Variant(void * p_ptr) 
{
	m_type = POINTER;
	m_data.m_ptr = p_ptr;
}

Variant::Variant(const Variant& p_variant) {

	m_type = NIL;
	reference(p_variant);
}

uint32_t Variant::hash() const 
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

bool Variant::booleanize() const 
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
