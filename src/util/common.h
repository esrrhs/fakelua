#pragma once

// Must be first: on UCRT+GCC 15, standard-library headers (e.g. <string>, <thread>)
// use errno internally without including <errno.h> themselves. UCRT defines errno as
// (*_errno()), so _errno() must be declared before those headers are parsed.
#include <cerrno>

#include <algorithm>
#include <assert.h>
#include <cassert>
#include <chrono>
#include <concepts>
#include <condition_variable>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <list>
#include <map>
#include <math.h>
#include <memory>
#include <mutex>
#include <numeric>
#include <pthread.h>
#include <queue>
#include <random>
#include <regex>
#include <set>
#include <shared_mutex>
#include <signal.h>
#include <source_location>
#include <sstream>
#include <stack>
#include <stdarg.h>
#include <stdexcept>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <string_view>
#include <thread>
#include <time.h>
#include <typeinfo>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>
#include <charconv>
#include <ranges>
#include <cctype>
#ifndef _WIN32
#include <execinfo.h>
#endif

#include "macro.h"
#include "debug.h"
#include "exception.h"
#include "logging.h"
#include "string_util.h"
#include "os.h"
#include "file_util.h"
#include "hash_func.h"
