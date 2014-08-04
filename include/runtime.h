#pragma once

#include "types.h"
#include "semantic.h"
#include "instructions.h"
#include <string>
#include <map>
#include <cstdlib>
#include <ctime>
#include <vector>

namespace fscript
{

class runtime
{
private:
    runtime();

public:
    static runtime* get_instance();

    void add_function_declaration(function_declaration* fd);

    /* Main hierarchy block */
    void set_main_block(block* block);
    block* get_main_block();

    /*Abstract semantic tree analys */
    bool semantic_analys();
    /* Interpretate */
    void interpretate();
    
private:
    /* Singletone realization */
    static runtime* self;
};

}

