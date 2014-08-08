#include "fuck.h"

/* Main hierarchy block */
void fuck::set_main_block(block* block)
{
    FKLOG("set_main_block");
}

block* fuck::get_main_block()
{
    return 0;
}


/*Abstract semantic tree analys */
bool fuck::semantic_analys()
{
    return true;
}


/* Interpretate */
void fuck::interpretate()
{
}

/*Function declaration operations*/
function_declaration * fuck::get_function_declaration(String name)
{   
    return 0;
}

void fuck::add_function_declaration(function_declaration* fd)
{    
    FKLOG("add_function_declaration");
}

efkerror fuck::error()
{
    return m_efkerror;
}

