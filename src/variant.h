#pragma once

#include "types.h"

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
    
    
	variant(const variant& p_variant);
	variant() { m_type = NIL; }
	~variant() { if (m_type != variant::NIL) clear(); }

	variant(bool p_bool);
	variant(signed int p_int); // real one
	variant(unsigned int p_int);
	variant(signed short p_short); // real one
	variant(unsigned short p_short);
	variant(signed char p_char); // real one
	variant(unsigned char p_char);
	variant(int64_t p_char); // real one
	variant(uint64_t p_char);
	variant(float p_float);
	variant(double p_double);
	variant(const String& p_string);
	variant(const char * const p_cstring);
	variant(void * p_ptr);
	
	bool operator==(const variant& p_variant) const;	
	void operator=(const variant& p_variant); // only this is enough for all the other types
	
	operator bool() const;
	operator signed int() const;
	operator unsigned int() const; // this is the real one
	operator signed short() const;
	operator unsigned short() const;
	operator signed char() const;
	operator unsigned char() const;
	//operator long unsigned int() const;
	operator int64_t() const;
	operator uint64_t() const;
	operator signed long() const;
	operator unsigned long() const;
	operator float() const;
	operator double() const;
	operator String() const;

	Type get_type() const { return m_type; }
	static String get_type_name(variant::Type p_type);

	uint32_t hash() const;

	bool booleanize() const;

private:

	union MemData
	{
		bool m_bool;
		int64_t m_int;
		double m_real;
		String * m_str;
		void * m_ptr; //generic pointer
	};

	void clear();
	void reference(const variant& p_variant);
	
private:

	Type m_type;

	MemData m_data;
};

