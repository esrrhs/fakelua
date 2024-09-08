#pragma once

namespace fakelua {

class var;
typedef var *(*VAR_FUNC)(...);

inline var *call_var_func(VAR_FUNC func, const std::vector<var *> &args) {
    switch (args.size()) {
        case 0:
            return func();
        case 1:
            return func(args[0]);
        case 2:
            return func(args[0], args[1]);
        case 3:
            return func(args[0], args[1], args[2]);
        case 4:
            return func(args[0], args[1], args[2], args[3]);
        case 5:
            return func(args[0], args[1], args[2], args[3], args[4]);
        case 6:
            return func(args[0], args[1], args[2], args[3], args[4], args[5]);
        case 7:
            return func(args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
        case 8:
            return func(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
        case 9:
            return func(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8]);
        case 10:
            return func(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9]);
        case 11:
            return func(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10]);
        case 12:
            return func(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11]);
        case 13:
            return func(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11],
                        args[12]);
        case 14:
            return func(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11],
                        args[12], args[13]);
        case 15:
            return func(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11],
                        args[12], args[13], args[14]);
        case 16:
            return func(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11],
                        args[12], args[13], args[14], args[15]);
        case 17:
            return func(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11],
                        args[12], args[13], args[14], args[15], args[16]);
        case 18:
            return func(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11],
                        args[12], args[13], args[14], args[15], args[16], args[17]);
        case 19:
            return func(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11],
                        args[12], args[13], args[14], args[15], args[16], args[17], args[18]);
        case 20:
            return func(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11],
                        args[12], args[13], args[14], args[15], args[16], args[17], args[18], args[19]);
        case 21:
            return func(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11],
                        args[12], args[13], args[14], args[15], args[16], args[17], args[18], args[19], args[20]);
        case 22:
            return func(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11],
                        args[12], args[13], args[14], args[15], args[16], args[17], args[18], args[19], args[20], args[21]);
        case 23:
            return func(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11],
                        args[12], args[13], args[14], args[15], args[16], args[17], args[18], args[19], args[20], args[21], args[22]);
        case 24:
            return func(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11],
                        args[12], args[13], args[14], args[15], args[16], args[17], args[18], args[19], args[20], args[21], args[22],
                        args[23]);
        case 25:
            return func(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11],
                        args[12], args[13], args[14], args[15], args[16], args[17], args[18], args[19], args[20], args[21], args[22],
                        args[23], args[24]);
        case 26:
            return func(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11],
                        args[12], args[13], args[14], args[15], args[16], args[17], args[18], args[19], args[20], args[21], args[22],
                        args[23], args[24], args[25]);
        case 27:
            return func(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11],
                        args[12], args[13], args[14], args[15], args[16], args[17], args[18], args[19], args[20], args[21], args[22],
                        args[23], args[24], args[25], args[26]);
        case 28:
            return func(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11],
                        args[12], args[13], args[14], args[15], args[16], args[17], args[18], args[19], args[20], args[21], args[22],
                        args[23], args[24], args[25], args[26], args[27]);
        case 29:
            return func(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11],
                        args[12], args[13], args[14], args[15], args[16], args[17], args[18], args[19], args[20], args[21], args[22],
                        args[23], args[24], args[25], args[26], args[27], args[28]);
        case 30:
            return func(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11],
                        args[12], args[13], args[14], args[15], args[16], args[17], args[18], args[19], args[20], args[21], args[22],
                        args[23], args[24], args[25], args[26], args[27], args[28], args[29]);
        default:
            throw_fakelua_exception(std::format("too many arguments: {}", args.size()));
    }
}

}// namespace fakelua
