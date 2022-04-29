#pragma once
#include "koopa.h"
#include<iostream>
#include<string>
#include<memory>
using namespace std;

void AST_2_KoopaIR(const char * str)
{
    // 解析字符串 str, 得到 Koopa IR 程序
    // 如果出现错误返回error code, 否则放回success code并将解析好的Koopa IR程序包存在program之中
    koopa_program_t program;
    koopa_error_code_t ret = koopa_parse_from_string(str, &program);
    assert(ret == KOOPA_EC_SUCCESS);  // 确保解析时没有出错

    // 创建一个 raw program builder, 用来构建 raw program, 指针类型，需要delete
    koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
    // 将 Koopa IR 程序转换为 raw program, row praogram是一个包含两个row slices的struct, 分别是values和funcs
    koopa_raw_program_t raw = koopa_build_raw_program(builder, program);

    // 释放 Koopa IR 程序占用的内存
    koopa_delete_program(program);

    // 处理 raw program
    // ...

    // 处理完成, 释放 raw program builder 占用的内存
    // 注意, raw program 中所有的指针指向的内存均为 raw program builder 的内存
    // 所以不要在 raw program 处理完毕之前释放 builder
    koopa_delete_raw_program_builder(builder);

   
}