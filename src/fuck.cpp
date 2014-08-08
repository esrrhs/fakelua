#include "fuck.h"

/* Main hierarchy block */
void fuck::set_main_block(block* block)
{
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
}

efkerror fuck::error()
{
    return m_efkerror;
}

