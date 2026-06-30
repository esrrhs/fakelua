#include "benchmark/benchmark.h"
#include "fakelua.h"

#include <lua.hpp>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

using namespace fakelua;

namespace {

constexpr const char *kFibScript = R"(
local function fib(n)
    if n <= 1 then
        return n
    end
    return fib(n - 1) + fib(n - 2)
end

function bench_fib(n)
    return fib(n)
end
)";

constexpr const char *kGcdScript = R"(
function bench_gcd(a, b)
    while b ~= 0 do
        a, b = b, a % b
    end
    return a
end
)";

constexpr const char *kPowScript = R"(
function bench_powmod(base, exp, mod)
    local result = 1
    base = base % mod
    while exp > 0 do
        if exp % 2 == 1 then
            result = (result * base) % mod
        end
        base = (base * base) % mod
        exp = exp // 2
    end
    return result
end
)";

constexpr const char *kSumScript = R"(
function bench_sum(n)
    local s = 0
    for i = 1, n do
        s = s + i
    end
    return s
end
)";

constexpr const char *kBubbleSortScript = R"(
local init_vals = { 200, 199, 198, 197, 196, 195, 194, 193, 192, 191, 190, 189, 188, 187, 186, 185, 184, 183, 182, 181, 180, 179, 178, 177, 176, 175, 174, 173, 172, 171, 170, 169, 168, 167, 166, 165, 164, 163, 162, 161, 160, 159, 158, 157, 156, 155, 154, 153, 152, 151, 150, 149, 148, 147, 146, 145, 144, 143, 142, 141, 140, 139, 138, 137, 136, 135, 134, 133, 132, 131, 130, 129, 128, 127, 126, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116, 115, 114, 113, 112, 111, 110, 109, 108, 107, 106, 105, 104, 103, 102, 101, 100, 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 }

function bench_bubble_sort(n)
    local t = {}
    local offset = 200 - n
    for i = 1, n do
        t[i] = init_vals[offset + i]
    end
    for i = 1, n do
        for j = 1, n - i do
            if t[j] > t[j + 1] then
                local tmp = t[j]
                t[j] = t[j + 1]
                t[j + 1] = tmp
            end
        end
    end
    return t[1]
end
)";
constexpr const char *kSieveScript = R"(
function bench_sieve(n)
    local is_prime = {}
    for i = 2, n do is_prime[i] = true end
    local i = 2
    while i * i <= n do
        if is_prime[i] then
            local j = i * i
            while j <= n do
                is_prime[j] = false
                j = j + i
            end
        end
        i = i + 1
    end
    local count = 0
    for k = 2, n do
        if is_prime[k] then
            count = count + 1
        end
    end
    return count
end
)";

constexpr const char *kBinarySearchScript = R"(
local search_init_vals = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 334, 335, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 348, 349, 350, 351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397, 398, 399, 400, 401, 402, 403, 404, 405, 406, 407, 408, 409, 410, 411, 412, 413, 414, 415, 416, 417, 418, 419, 420, 421, 422, 423, 424, 425, 426, 427, 428, 429, 430, 431, 432, 433, 434, 435, 436, 437, 438, 439, 440, 441, 442, 443, 444, 445, 446, 447, 448, 449, 450, 451, 452, 453, 454, 455, 456, 457, 458, 459, 460, 461, 462, 463, 464, 465, 466, 467, 468, 469, 470, 471, 472, 473, 474, 475, 476, 477, 478, 479, 480, 481, 482, 483, 484, 485, 486, 487, 488, 489, 490, 491, 492, 493, 494, 495, 496, 497, 498, 499, 500, 501, 502, 503, 504, 505, 506, 507, 508, 509, 510, 511, 512, 513, 514, 515, 516, 517, 518, 519, 520, 521, 522, 523, 524, 525, 526, 527, 528, 529, 530, 531, 532, 533, 534, 535, 536, 537, 538, 539, 540, 541, 542, 543, 544, 545, 546, 547, 548, 549, 550, 551, 552, 553, 554, 555, 556, 557, 558, 559, 560, 561, 562, 563, 564, 565, 566, 567, 568, 569, 570, 571, 572, 573, 574, 575, 576, 577, 578, 579, 580, 581, 582, 583, 584, 585, 586, 587, 588, 589, 590, 591, 592, 593, 594, 595, 596, 597, 598, 599, 600, 601, 602, 603, 604, 605, 606, 607, 608, 609, 610, 611, 612, 613, 614, 615, 616, 617, 618, 619, 620, 621, 622, 623, 624, 625, 626, 627, 628, 629, 630, 631, 632, 633, 634, 635, 636, 637, 638, 639, 640, 641, 642, 643, 644, 645, 646, 647, 648, 649, 650, 651, 652, 653, 654, 655, 656, 657, 658, 659, 660, 661, 662, 663, 664, 665, 666, 667, 668, 669, 670, 671, 672, 673, 674, 675, 676, 677, 678, 679, 680, 681, 682, 683, 684, 685, 686, 687, 688, 689, 690, 691, 692, 693, 694, 695, 696, 697, 698, 699, 700, 701, 702, 703, 704, 705, 706, 707, 708, 709, 710, 711, 712, 713, 714, 715, 716, 717, 718, 719, 720, 721, 722, 723, 724, 725, 726, 727, 728, 729, 730, 731, 732, 733, 734, 735, 736, 737, 738, 739, 740, 741, 742, 743, 744, 745, 746, 747, 748, 749, 750, 751, 752, 753, 754, 755, 756, 757, 758, 759, 760, 761, 762, 763, 764, 765, 766, 767, 768, 769, 770, 771, 772, 773, 774, 775, 776, 777, 778, 779, 780, 781, 782, 783, 784, 785, 786, 787, 788, 789, 790, 791, 792, 793, 794, 795, 796, 797, 798, 799, 800, 801, 802, 803, 804, 805, 806, 807, 808, 809, 810, 811, 812, 813, 814, 815, 816, 817, 818, 819, 820, 821, 822, 823, 824, 825, 826, 827, 828, 829, 830, 831, 832, 833, 834, 835, 836, 837, 838, 839, 840, 841, 842, 843, 844, 845, 846, 847, 848, 849, 850, 851, 852, 853, 854, 855, 856, 857, 858, 859, 860, 861, 862, 863, 864, 865, 866, 867, 868, 869, 870, 871, 872, 873, 874, 875, 876, 877, 878, 879, 880, 881, 882, 883, 884, 885, 886, 887, 888, 889, 890, 891, 892, 893, 894, 895, 896, 897, 898, 899, 900, 901, 902, 903, 904, 905, 906, 907, 908, 909, 910, 911, 912, 913, 914, 915, 916, 917, 918, 919, 920, 921, 922, 923, 924, 925, 926, 927, 928, 929, 930, 931, 932, 933, 934, 935, 936, 937, 938, 939, 940, 941, 942, 943, 944, 945, 946, 947, 948, 949, 950, 951, 952, 953, 954, 955, 956, 957, 958, 959, 960, 961, 962, 963, 964, 965, 966, 967, 968, 969, 970, 971, 972, 973, 974, 975, 976, 977, 978, 979, 980, 981, 982, 983, 984, 985, 986, 987, 988, 989, 990, 991, 992, 993, 994, 995, 996, 997, 998, 999, 1000 }

function bench_binary_search(n)
    local found = 0
    for target = 1, n do
        local lo = 1
        local hi = n
        while lo <= hi do
            local mid = (lo + hi) // 2
            if search_init_vals[mid] == target then
                found = found + 1
                break
            elseif search_init_vals[mid] < target then
                lo = mid + 1
            else
                hi = mid - 1
            end
        end
    end
    return found
end
)";
constexpr const char *kFastPowScript = R"(
function bench_fast_pow(base, exp, mod)
    local result = 1
    base = base % mod
    while exp > 0 do
        if (exp & 1) == 1 then
            result = (result * base) % mod
        end
        exp = exp >> 1
        base = (base * base) % mod
    end
    return result
end
)";

constexpr const char *kPopcountScript = R"(
function bench_popcount(n)
    local total = 0
    for i = 0, n - 1 do
        local x = i
        while x ~= 0 do
            x = x & (x - 1)
            total = total + 1
        end
    end
    return total
end
)";

constexpr const char *kInsertionSortScript = R"(
local init_vals = { 200, 199, 198, 197, 196, 195, 194, 193, 192, 191, 190, 189, 188, 187, 186, 185, 184, 183, 182, 181, 180, 179, 178, 177, 176, 175, 174, 173, 172, 171, 170, 169, 168, 167, 166, 165, 164, 163, 162, 161, 160, 159, 158, 157, 156, 155, 154, 153, 152, 151, 150, 149, 148, 147, 146, 145, 144, 143, 142, 141, 140, 139, 138, 137, 136, 135, 134, 133, 132, 131, 130, 129, 128, 127, 126, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116, 115, 114, 113, 112, 111, 110, 109, 108, 107, 106, 105, 104, 103, 102, 101, 100, 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 }

function bench_insertion_sort(n)
    local t = {}
    local offset = 200 - n
    for i = 1, n do
        t[i] = init_vals[offset + i]
    end
    for i = 2, n do
        local key = t[i]
        local j = i - 1
        while j >= 1 and t[j] > key do
            t[j + 1] = t[j]
            j = j - 1
        end
        t[j + 1] = key
    end
    return t[1]
end
)";
constexpr const char *kMatMulScript = R"(
local mat_a = {1, 2, 3, 4, 5, 6, 7, 8, 9}
local mat_b = {9, 8, 7, 6, 5, 4, 3, 2, 1}
local mat_c_init = {0, 0, 0, 0, 0, 0, 0, 0, 0}

function bench_matmul()
    local c = {}
    for i = 1, 9 do
        c[i] = mat_c_init[i]
    end
    for i = 1, 3 do
        for j = 1, 3 do
            local s = 0
            for k = 1, 3 do
                s = s + mat_a[(i - 1) * 3 + k] * mat_b[(k - 1) * 3 + j]
            end
            c[(i - 1) * 3 + j] = s
        end
    end
    return c[1] + c[5] + c[9]
end
)";
constexpr const char *kVector3Script = R"(
function bench_vector3(n)
    local v1 = {x = 10, y = 20, z = 30}
    local v2 = {x = 1, y = 2, z = 3}
    local sum = 0
    for i = 1, n do
        v1.x = v1.x + v2.x
        v1.y = v1.y + v2.y
        v1.z = v1.z + v2.z
        sum = sum + v1.x + v1.y + v1.z
    end
    return sum
end
)";
int64_t CppFib(const int64_t n) {
    if (n <= 1) {
        return n;
    }
    return CppFib(n - 1) + CppFib(n - 2);
}

int64_t CppGcd(int64_t a, int64_t b) {
    while (b != 0) {
        const int64_t t = a % b;
        a = b;
        b = t;
    }
    return a;
}

int64_t CppPowMod(int64_t base, int64_t exp, const int64_t mod) {
    int64_t result = 1;
    base %= mod;
    while (exp > 0) {
        if ((exp & 1) != 0) {
            result = (result * base) % mod;
        }
        base = (base * base) % mod;
        exp >>= 1;
    }
    return result;
}

int64_t CppSum(const int64_t n) {
    int64_t s = 0;
    for (int64_t i = 1; i <= n; ++i) {
        s += i;
    }
    return s;
}

// A global volatile zero prevents the compiler from constant-folding the 3x3
// matrix product in CppMatMul while keeping the numeric results correct.
volatile int64_t g_matmul_nonce = 0;

int64_t CppVector3(int64_t n) {
    struct Vector3 {
        int64_t x;
        int64_t y;
        int64_t z;
    };
    Vector3 v1{10 + g_matmul_nonce, 20, 30};
    Vector3 v2{1, 2, 3};
    int64_t sum = 0;
    for (int64_t i = 1; i <= n; ++i) {
        v1.x = v1.x + v2.x;
        v1.y = v1.y + v2.y;
        v1.z = v1.z + v2.z;
        sum = sum + v1.x + v1.y + v1.z;
    }
    return sum;
}

int64_t CppBubbleSort(const int64_t n) {
    std::vector<int64_t> t(static_cast<size_t>(n));
    for (int64_t i = 0; i < n; ++i) {
        t[static_cast<size_t>(i)] = n - i;
    }
    for (int64_t i = 1; i <= n; ++i) {
        for (int64_t j = 1; j <= n - i; ++j) {
            if (t[static_cast<size_t>(j - 1)] > t[static_cast<size_t>(j)]) {
                std::swap(t[static_cast<size_t>(j - 1)], t[static_cast<size_t>(j)]);
            }
        }
    }
    return t[0];
}

int64_t CppSieve(const int64_t n) {
    std::vector<bool> is_prime(static_cast<size_t>(n + 1), true);
    for (int64_t i = 2; i * i <= n; ++i) {
        if (is_prime[static_cast<size_t>(i)]) {
            for (int64_t j = i * i; j <= n; j += i) {
                is_prime[static_cast<size_t>(j)] = false;
            }
        }
    }
    int64_t count = 0;
    for (int64_t k = 2; k <= n; ++k) {
        if (is_prime[static_cast<size_t>(k)]) {
            ++count;
        }
    }
    return count;
}

int64_t CppBinarySearch(const int64_t n) {
    std::vector<int64_t> t(static_cast<size_t>(n));
    for (int64_t i = 0; i < n; ++i) {
        t[static_cast<size_t>(i)] = i + 1;
    }
    int64_t found = 0;
    for (int64_t target = 1; target <= n; ++target) {
        int64_t lo = 0;
        int64_t hi = n - 1;
        while (lo <= hi) {
            const int64_t mid = (lo + hi) / 2;
            if (t[static_cast<size_t>(mid)] == target) {
                ++found;
                break;
            } else if (t[static_cast<size_t>(mid)] < target) {
                lo = mid + 1;
            } else {
                hi = mid - 1;
            }
        }
    }
    return found;
}

int64_t CppFastPow(int64_t base, int64_t exp, const int64_t mod) {
    int64_t result = 1;
    base %= mod;
    while (exp > 0) {
        if ((exp & 1) != 0) {
            result = (result * base) % mod;
        }
        exp >>= 1;
        base = (base * base) % mod;
    }
    return result;
}

int64_t CppPopcount(const int64_t n) {
    int64_t total = 0;
    for (int64_t i = 0; i < n; ++i) {
        int64_t x = i;
        while (x != 0) {
            x &= (x - 1);
            ++total;
        }
    }
    return total;
}

int64_t CppInsertionSort(const int64_t n) {
    std::vector<int64_t> t(static_cast<size_t>(n));
    for (int64_t i = 0; i < n; ++i) {
        t[static_cast<size_t>(i)] = n - i;
    }
    for (int64_t i = 1; i < n; ++i) {
        const int64_t key = t[static_cast<size_t>(i)];
        int64_t j = i - 1;
        while (j >= 0 && t[static_cast<size_t>(j)] > key) {
            t[static_cast<size_t>(j + 1)] = t[static_cast<size_t>(j)];
            --j;
        }
        t[static_cast<size_t>(j + 1)] = key;
    }
    return t[0];
}

int64_t CppMatMul() {
    // a[0] depends on a volatile global (always 1 at runtime) so the compiler
    // cannot precompute the matrix product at compile time.
    int64_t a[9] = {g_matmul_nonce + 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int64_t b[9] = {9, 8, 7, 6, 5, 4, 3, 2, 1};
    int64_t c[9] = {};
    for (int64_t i = 0; i < 3; ++i) {
        for (int64_t j = 0; j < 3; ++j) {
            int64_t s = 0;
            for (int64_t k = 0; k < 3; ++k) {
                s += a[i * 3 + k] * b[k * 3 + j];
            }
            c[i * 3 + j] = s;
        }
    }
    return c[0] + c[4] + c[8];
}

void PushLuaArg(lua_State *L, const int64_t value) {
    lua_pushinteger(L, static_cast<lua_Integer>(value));
}

void PushLuaArgs(lua_State *) {
}

template<typename T, typename... Args>
void PushLuaArgs(lua_State *L, T first, Args... args) {
    PushLuaArg(L, static_cast<int64_t>(first));
    PushLuaArgs(L, args...);
}

template<typename... Args>
int64_t CallLuaInt(lua_State *L, const char *func_name, Args... args) {
    const int top = lua_gettop(L);
    lua_getglobal(L, func_name);
    if (!lua_isfunction(L, -1)) {
        lua_settop(L, top);
        throw std::runtime_error(std::string("Lua function not found: ") + func_name);
    }

    PushLuaArgs(L, args...);

    constexpr int nargs = sizeof...(Args);
    if (const int code = lua_pcall(L, nargs, 1, 0); code != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        std::string msg = err ? err : "unknown lua error";
        lua_settop(L, top);
        throw std::runtime_error("Lua call failed: " + msg);
    }

    const auto ret = static_cast<int64_t>(lua_tointeger(L, -1));
    lua_settop(L, top);
    return ret;
}

struct RuntimeContext {
    RuntimeContext() {
        lua = luaL_newstate();
        luaL_openlibs(lua);

        const char *lua_scripts[] = {
                kFibScript,    kGcdScript,    kPowScript,         kSumScript,        kBubbleSortScript,
                kSieveScript,  kBinarySearchScript, kFastPowScript, kPopcountScript,
                kInsertionSortScript, kMatMulScript, kVector3Script};
        for (const char *script : lua_scripts) {
            if (luaL_dostring(lua, script) != LUA_OK) {
                const char *err = lua_tostring(lua, -1);
                throw std::runtime_error(std::string("init lua scripts failed: ") + (err ? err : "unknown"));
            }
        }

        flua = FakeluaNewState();
        for (const char *script : lua_scripts) {
            CompileString(flua, script, {.debug_mode = false});
        }

        // Warmup: call each FakeLua function once to page in JIT code and
        // trigger any lazy initialization before benchmarks start timing.
        int64_t warmup_ret = 0;
        Call(flua, JIT_TCC, "bench_fib", warmup_ret, 10);
        Call(flua, JIT_TCC, "bench_gcd", warmup_ret, 832040, 514229);
        Call(flua, JIT_TCC, "bench_powmod", warmup_ret, 2, 1000, 1000000007);
        Call(flua, JIT_TCC, "bench_sum", warmup_ret, 100);
        Call(flua, JIT_TCC, "bench_bubble_sort", warmup_ret, 10);
        Call(flua, JIT_TCC, "bench_sieve", warmup_ret, 100);
        Call(flua, JIT_TCC, "bench_binary_search", warmup_ret, 100);
        Call(flua, JIT_TCC, "bench_fast_pow", warmup_ret, 2, 1000, 1000000007);
        Call(flua, JIT_TCC, "bench_popcount", warmup_ret, 100);
        Call(flua, JIT_TCC, "bench_insertion_sort", warmup_ret, 10);
        Call(flua, JIT_TCC, "bench_matmul", warmup_ret);
        Call(flua, JIT_TCC, "bench_vector3", warmup_ret, 100);
        Call(flua, JIT_GCC, "bench_fib", warmup_ret, 10);
        Call(flua, JIT_GCC, "bench_gcd", warmup_ret, 832040, 514229);
        Call(flua, JIT_GCC, "bench_powmod", warmup_ret, 2, 1000, 1000000007);
        Call(flua, JIT_GCC, "bench_sum", warmup_ret, 100);
        Call(flua, JIT_GCC, "bench_bubble_sort", warmup_ret, 10);
        Call(flua, JIT_GCC, "bench_sieve", warmup_ret, 100);
        Call(flua, JIT_GCC, "bench_binary_search", warmup_ret, 100);
        Call(flua, JIT_GCC, "bench_fast_pow", warmup_ret, 2, 1000, 1000000007);
        Call(flua, JIT_GCC, "bench_popcount", warmup_ret, 100);
        Call(flua, JIT_GCC, "bench_insertion_sort", warmup_ret, 10);
        Call(flua, JIT_GCC, "bench_matmul", warmup_ret);
        Call(flua, JIT_GCC, "bench_vector3", warmup_ret, 100);
    }

    ~RuntimeContext() {
        if (lua) {
            lua_close(lua);
            lua = nullptr;
        }
        if (flua) {
            FakeluaDeleteState(flua);
            flua = nullptr;
        }
    }

    lua_State *lua = nullptr;
    State *flua = nullptr;
};

RuntimeContext g_ctx;

void VerifyEqual(const int64_t got, const int64_t expected, const char *name) {
    if (got != expected) {
        throw std::runtime_error(std::string(name) + " wrong result: got " + std::to_string(got) +
                                 ", expected " + std::to_string(expected));
    }
}

static void BM_CPP_Fibonacci(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppFib(n);
    for ([[maybe_unused]] auto _: state) {
        int64_t input = n;
        benchmark::DoNotOptimize(input);
        int64_t ret = CppFib(input);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ fib");
    }
}

static void BM_Lua_Fibonacci(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppFib(n);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_fib", n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "Lua fib");
    }
}

static void BM_FakeLua_Fibonacci_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppFib(n);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_fib", ret, n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua fib");
    }
}

static void BM_FakeLua_Fibonacci_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppFib(n);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_fib", ret, n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua fib");
    }
}

static void BM_CPP_GCD(benchmark::State &state) {
    const int64_t a = state.range(0);
    const int64_t b = state.range(1);
    const int64_t expected = CppGcd(a, b);
    for ([[maybe_unused]] auto _: state) {
        int64_t aa = a;
        int64_t bb = b;
        benchmark::DoNotOptimize(aa);
        benchmark::DoNotOptimize(bb);
        int64_t ret = CppGcd(aa, bb);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ gcd");
    }
}

static void BM_Lua_GCD(benchmark::State &state) {
    const int64_t a = state.range(0);
    const int64_t b = state.range(1);
    const int64_t expected = CppGcd(a, b);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_gcd", a, b);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "Lua gcd");
    }
}

static void BM_FakeLua_GCD_TCC(benchmark::State &state) {
    const int64_t a = state.range(0);
    const int64_t b = state.range(1);
    const int64_t expected = CppGcd(a, b);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_gcd", ret, a, b);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua gcd");
    }
}

static void BM_FakeLua_GCD_GCC(benchmark::State &state) {
    const int64_t a = state.range(0);
    const int64_t b = state.range(1);
    const int64_t expected = CppGcd(a, b);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_gcd", ret, a, b);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua gcd");
    }
}

static void BM_CPP_PowMod(benchmark::State &state) {
    const int64_t base = state.range(0);
    const int64_t exp = state.range(1);
    const int64_t mod = state.range(2);
    const int64_t expected = CppPowMod(base, exp, mod);
    for ([[maybe_unused]] auto _: state) {
        int64_t bb = base;
        int64_t ee = exp;
        int64_t mm = mod;
        benchmark::DoNotOptimize(bb);
        benchmark::DoNotOptimize(ee);
        benchmark::DoNotOptimize(mm);
        int64_t ret = CppPowMod(bb, ee, mm);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ powmod");
    }
}

static void BM_Lua_PowMod(benchmark::State &state) {
    const int64_t base = state.range(0);
    const int64_t exp = state.range(1);
    const int64_t mod = state.range(2);
    const int64_t expected = CppPowMod(base, exp, mod);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_powmod", base, exp, mod);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "Lua powmod");
    }
}

static void BM_FakeLua_PowMod_TCC(benchmark::State &state) {
    const int64_t base = state.range(0);
    const int64_t exp = state.range(1);
    const int64_t mod = state.range(2);
    const int64_t expected = CppPowMod(base, exp, mod);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_powmod", ret, base, exp, mod);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua powmod");
    }
}

static void BM_FakeLua_PowMod_GCC(benchmark::State &state) {
    const int64_t base = state.range(0);
    const int64_t exp = state.range(1);
    const int64_t mod = state.range(2);
    const int64_t expected = CppPowMod(base, exp, mod);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_powmod", ret, base, exp, mod);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua powmod");
    }
}

static void BM_CPP_Sum(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppSum(n);
    for ([[maybe_unused]] auto _: state) {
        int64_t input = n;
        benchmark::DoNotOptimize(input);
        int64_t ret = CppSum(input);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ sum");
    }
}

static void BM_Lua_Sum(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppSum(n);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_sum", n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "Lua sum");
    }
}

static void BM_FakeLua_Sum_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppSum(n);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_sum", ret, n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua sum");
    }
}

static void BM_FakeLua_Sum_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppSum(n);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_sum", ret, n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua sum");
    }
}

static void BM_CPP_BubbleSort(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = 1;
    for ([[maybe_unused]] auto _: state) {
        int64_t input = n;
        benchmark::DoNotOptimize(input);
        int64_t ret = CppBubbleSort(input);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ bubble_sort");
    }
}

static void BM_Lua_BubbleSort(benchmark::State &state) {
    const int64_t n = state.range(0);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_bubble_sort", n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, 1, "Lua bubble_sort");
    }
}

static void BM_FakeLua_BubbleSort_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_bubble_sort", ret, n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, 1, "FakeLua bubble_sort");
    }
}

static void BM_FakeLua_BubbleSort_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_bubble_sort", ret, n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, 1, "FakeLua bubble_sort");
    }
}

static void BM_CPP_Sieve(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppSieve(n);
    for ([[maybe_unused]] auto _: state) {
        int64_t input = n;
        benchmark::DoNotOptimize(input);
        int64_t ret = CppSieve(input);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ sieve");
    }
}

static void BM_Lua_Sieve(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppSieve(n);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_sieve", n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "Lua sieve");
    }
}

static void BM_FakeLua_Sieve_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppSieve(n);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_sieve", ret, n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua sieve");
    }
}

static void BM_FakeLua_Sieve_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppSieve(n);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_sieve", ret, n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua sieve");
    }
}

static void BM_CPP_BinarySearch(benchmark::State &state) {
    const int64_t n = state.range(0);
    for ([[maybe_unused]] auto _: state) {
        int64_t input = n;
        benchmark::DoNotOptimize(input);
        int64_t ret = CppBinarySearch(input);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, n, "C++ binary_search");
    }
}

static void BM_Lua_BinarySearch(benchmark::State &state) {
    const int64_t n = state.range(0);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_binary_search", n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, n, "Lua binary_search");
    }
}

static void BM_FakeLua_BinarySearch_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_binary_search", ret, n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, n, "FakeLua binary_search");
    }
}

static void BM_FakeLua_BinarySearch_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_binary_search", ret, n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, n, "FakeLua binary_search");
    }
}

static void BM_CPP_FastPow(benchmark::State &state) {
    const int64_t base = state.range(0);
    const int64_t exp = state.range(1);
    const int64_t mod = state.range(2);
    const int64_t expected = CppFastPow(base, exp, mod);
    for ([[maybe_unused]] auto _: state) {
        int64_t bb = base;
        int64_t ee = exp;
        int64_t mm = mod;
        benchmark::DoNotOptimize(bb);
        benchmark::DoNotOptimize(ee);
        benchmark::DoNotOptimize(mm);
        int64_t ret = CppFastPow(bb, ee, mm);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ fast_pow");
    }
}

static void BM_Lua_FastPow(benchmark::State &state) {
    const int64_t base = state.range(0);
    const int64_t exp = state.range(1);
    const int64_t mod = state.range(2);
    const int64_t expected = CppFastPow(base, exp, mod);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_fast_pow", base, exp, mod);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "Lua fast_pow");
    }
}

static void BM_FakeLua_FastPow_TCC(benchmark::State &state) {
    const int64_t base = state.range(0);
    const int64_t exp = state.range(1);
    const int64_t mod = state.range(2);
    const int64_t expected = CppFastPow(base, exp, mod);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_fast_pow", ret, base, exp, mod);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua fast_pow");
    }
}

static void BM_FakeLua_FastPow_GCC(benchmark::State &state) {
    const int64_t base = state.range(0);
    const int64_t exp = state.range(1);
    const int64_t mod = state.range(2);
    const int64_t expected = CppFastPow(base, exp, mod);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_fast_pow", ret, base, exp, mod);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua fast_pow");
    }
}

static void BM_CPP_Popcount(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppPopcount(n);
    for ([[maybe_unused]] auto _: state) {
        int64_t input = n;
        benchmark::DoNotOptimize(input);
        int64_t ret = CppPopcount(input);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ popcount");
    }
}

static void BM_Lua_Popcount(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppPopcount(n);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_popcount", n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "Lua popcount");
    }
}

static void BM_FakeLua_Popcount_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppPopcount(n);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_popcount", ret, n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua popcount");
    }
}

static void BM_FakeLua_Popcount_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppPopcount(n);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_popcount", ret, n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua popcount");
    }
}

static void BM_CPP_InsertionSort(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = 1;
    for ([[maybe_unused]] auto _: state) {
        int64_t input = n;
        benchmark::DoNotOptimize(input);
        int64_t ret = CppInsertionSort(input);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ insertion_sort");
    }
}

static void BM_Lua_InsertionSort(benchmark::State &state) {
    const int64_t n = state.range(0);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_insertion_sort", n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, 1, "Lua insertion_sort");
    }
}

static void BM_FakeLua_InsertionSort_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_insertion_sort", ret, n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, 1, "FakeLua insertion_sort");
    }
}

static void BM_FakeLua_InsertionSort_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_insertion_sort", ret, n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, 1, "FakeLua insertion_sort");
    }
}

static void BM_CPP_MatMul(benchmark::State &state) {
    const int64_t expected = CppMatMul();
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = CppMatMul();
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ matmul");
    }
}

static void BM_Lua_MatMul(benchmark::State &state) {
    const int64_t expected = CppMatMul();
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_matmul");
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "Lua matmul");
    }
}

static void BM_FakeLua_MatMul_TCC(benchmark::State &state) {
    const int64_t expected = CppMatMul();
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_matmul", ret);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua matmul");
    }
}

static void BM_FakeLua_MatMul_GCC(benchmark::State &state) {
    const int64_t expected = CppMatMul();
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_matmul", ret);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua matmul");
    }
}

static void BM_CPP_Vector3(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppVector3(n);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = CppVector3(n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "C++ vector3");
    }
}

static void BM_Lua_Vector3(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppVector3(n);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = CallLuaInt(g_ctx.lua, "bench_vector3", n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "Lua vector3");
    }
}

static void BM_FakeLua_Vector3_TCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppVector3(n);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_TCC, "bench_vector3", ret, n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua vector3");
    }
}

static void BM_FakeLua_Vector3_GCC(benchmark::State &state) {
    const int64_t n = state.range(0);
    const int64_t expected = CppVector3(n);
    for ([[maybe_unused]] auto _: state) {
        int64_t ret = 0;
        Call(g_ctx.flua, JIT_GCC, "bench_vector3", ret, n);
        benchmark::DoNotOptimize(ret);
        VerifyEqual(ret, expected, "FakeLua vector3");
    }
}

}// namespace

#define FIB_ARGS ->Arg(20)->Arg(25)->Arg(30)->Arg(32)
#define GCD_ARGS ->Args({832040, 514229})->Args({123456789, 987654321})->Args({2147483647, 1073741823})
#define POWMOD_ARGS                                                                                                         \
    ->Args({2, 1000, 1000000007})->Args({7, 1000000, 1000000007})->Args({1234567, 7654321, 1000000007})
#define SUM_ARGS ->Arg(10000)->Arg(100000)->Arg(1000000)->Arg(5000000)
#define BUBBLE_SORT_ARGS ->Arg(50)->Arg(100)->Arg(200)
#define SIEVE_ARGS ->Arg(100)->Arg(500)->Arg(1000)->Arg(5000)
#define BINARY_SEARCH_ARGS ->Arg(100)->Arg(500)->Arg(1000)
#define FAST_POW_ARGS ->Args({2, 1000, 1000000007})->Args({7, 1000000, 1000000007})->Args({1234567, 7654321, 1000000007})
#define POPCOUNT_ARGS ->Arg(1000)->Arg(10000)->Arg(100000)
#define INSERTION_SORT_ARGS ->Arg(50)->Arg(100)->Arg(200)

BENCHMARK(BM_CPP_Fibonacci) FIB_ARGS;
BENCHMARK(BM_Lua_Fibonacci) FIB_ARGS;
BENCHMARK(BM_FakeLua_Fibonacci_TCC) FIB_ARGS;
BENCHMARK(BM_FakeLua_Fibonacci_GCC) FIB_ARGS;

BENCHMARK(BM_CPP_GCD) GCD_ARGS;
BENCHMARK(BM_Lua_GCD) GCD_ARGS;
BENCHMARK(BM_FakeLua_GCD_TCC) GCD_ARGS;
BENCHMARK(BM_FakeLua_GCD_GCC) GCD_ARGS;

BENCHMARK(BM_CPP_PowMod) POWMOD_ARGS;
BENCHMARK(BM_Lua_PowMod) POWMOD_ARGS;
BENCHMARK(BM_FakeLua_PowMod_TCC) POWMOD_ARGS;
BENCHMARK(BM_FakeLua_PowMod_GCC) POWMOD_ARGS;

BENCHMARK(BM_CPP_Sum) SUM_ARGS;
BENCHMARK(BM_Lua_Sum) SUM_ARGS;
BENCHMARK(BM_FakeLua_Sum_TCC) SUM_ARGS;
BENCHMARK(BM_FakeLua_Sum_GCC) SUM_ARGS;

BENCHMARK(BM_CPP_BubbleSort) BUBBLE_SORT_ARGS;
BENCHMARK(BM_Lua_BubbleSort) BUBBLE_SORT_ARGS;
BENCHMARK(BM_FakeLua_BubbleSort_TCC) BUBBLE_SORT_ARGS;
BENCHMARK(BM_FakeLua_BubbleSort_GCC) BUBBLE_SORT_ARGS;

BENCHMARK(BM_CPP_Sieve) SIEVE_ARGS;
BENCHMARK(BM_Lua_Sieve) SIEVE_ARGS;
BENCHMARK(BM_FakeLua_Sieve_TCC) SIEVE_ARGS;
BENCHMARK(BM_FakeLua_Sieve_GCC) SIEVE_ARGS;

BENCHMARK(BM_CPP_BinarySearch) BINARY_SEARCH_ARGS;
BENCHMARK(BM_Lua_BinarySearch) BINARY_SEARCH_ARGS;
BENCHMARK(BM_FakeLua_BinarySearch_TCC) BINARY_SEARCH_ARGS;
BENCHMARK(BM_FakeLua_BinarySearch_GCC) BINARY_SEARCH_ARGS;

BENCHMARK(BM_CPP_FastPow) FAST_POW_ARGS;
BENCHMARK(BM_Lua_FastPow) FAST_POW_ARGS;
BENCHMARK(BM_FakeLua_FastPow_TCC) FAST_POW_ARGS;
BENCHMARK(BM_FakeLua_FastPow_GCC) FAST_POW_ARGS;

BENCHMARK(BM_CPP_Popcount) POPCOUNT_ARGS;
BENCHMARK(BM_Lua_Popcount) POPCOUNT_ARGS;
BENCHMARK(BM_FakeLua_Popcount_TCC) POPCOUNT_ARGS;
BENCHMARK(BM_FakeLua_Popcount_GCC) POPCOUNT_ARGS;

BENCHMARK(BM_CPP_InsertionSort) INSERTION_SORT_ARGS;
BENCHMARK(BM_Lua_InsertionSort) INSERTION_SORT_ARGS;
BENCHMARK(BM_FakeLua_InsertionSort_TCC) INSERTION_SORT_ARGS;
BENCHMARK(BM_FakeLua_InsertionSort_GCC) INSERTION_SORT_ARGS;

BENCHMARK(BM_CPP_MatMul);
BENCHMARK(BM_Lua_MatMul);
BENCHMARK(BM_FakeLua_MatMul_TCC);
BENCHMARK(BM_FakeLua_MatMul_GCC);

#define VECTOR3_ARGS ->Arg(10000)->Arg(100000)->Arg(1000000)

BENCHMARK(BM_CPP_Vector3) VECTOR3_ARGS;
BENCHMARK(BM_Lua_Vector3) VECTOR3_ARGS;
BENCHMARK(BM_FakeLua_Vector3_TCC) VECTOR3_ARGS;
BENCHMARK(BM_FakeLua_Vector3_GCC) VECTOR3_ARGS;
