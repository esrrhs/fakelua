#pragma once

// 此文件为对外接口，尽量与lua接口保持一致
class fakelua_state;

enum fakelua_error {
    FAKELUA_OK,
    FAKELUA_FILE_FAIL,
};

// 创建虚拟机
extern "C" fakelua_state *fakelua_newstate();

// 销毁虚拟机
extern "C" void fakelua_close(fakelua_state *L);

// 加载执行文件
extern "C" int fakelua_dofile(fakelua_state *L, const char *file_path);

// 加载执行字符串
extern "C" int fakelua_dostring(fakelua_state *L, const char *content);
