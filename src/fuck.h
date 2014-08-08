#pragma once

#include <string>
#include <list>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <typeinfo>
#include <stdio.h>
#include "types.h"
#include "instructions.h"
#include "myflexer.h"
#include "fuckscript.h"

struct fuck
{
    fuck() : m_myflexer(this)
    {
        clear();
    }

    void clear()
    {
        m_myflexer.clear();
        m_efkerror = efk_ok;
    }
    
    /* Main hierarchy block */
    void set_main_block(block* block);
    block* get_main_block();

    /*Abstract semantic tree analys */
    bool semantic_analys();
    
    /* Interpretate */
    void interpretate();
    
    /*Function declaration operations*/
    function_declaration * get_function_declaration(String name);
    void add_function_declaration(function_declaration* fd);

    // error
    efkerror error();

    // member
    myflexer m_myflexer;
    efkerror m_efkerror;
};


