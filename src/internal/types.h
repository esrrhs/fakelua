#pragma once

#include <string>
#include <list>
#include <vector>
#include <map>
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
#include <time.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>
#include <fstream>
#include <filesystem>
#include <string_view>
#include <experimental/source_location>

void log(const std::string_view &message,
         const std::experimental::source_location &location = std::experimental::source_location::current());
