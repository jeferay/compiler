#pragma once
#include "koopa.h"
#include <iostream>
#include <string>
#include <cstring>
#include <memory>
using namespace std;


// 函数重载
void Visit(const koopa_raw_slice_t &slice, char * RiscV);
void Visit(const koopa_raw_return_t & ret, char * RiscV);
void Visit(const koopa_raw_integer_t &integer, char * RiscV);
void Visit(const koopa_raw_binary_t &binary, char * RiscV);
void Visit(const koopa_raw_value_t &value, char * RiscV);
void Visit(const koopa_raw_basic_block_t &bb, char * RiscV);
void Visit(const koopa_raw_function_t &func, char * RiscV);
void Visit(const koopa_raw_slice_t &slice, char * RiscV);
void Visit(const koopa_raw_program_t &program, char * RiscV);
void Str_2_KoopaIR(const char * IR, char * RiscV);



// 访问return指令
void Visit(const koopa_raw_return_t & ret, char * RiscV)
{
  koopa_raw_value_t value = ret.value;// 定义为一个const koopa_raw_value_data_t *
  if (value != NULL)
  {
    koopa_raw_value_kind_t kind = value->kind;
    int32_t ret_data = kind.data.integer.value;
    strcat(RiscV, "  li a0, ");
    strcat(RiscV, const_cast<char *>(to_string(ret_data).c_str()));
    strcat(RiscV, "\n");
    
  }
  strcat(RiscV, "  ret\n");
}

void Visit(const koopa_raw_binary_t &binary, char * RiscV)
{
  cout<<"visit_ins_binary in\n";
  koopa_raw_value_t lhs = binary.lhs; // *koopa_raw_value_data指针类型
  koopa_raw_value_t rhs = binary.rhs;
  cout<<binary.op<<" "<<(lhs->kind).data.integer.value<<" "<<(lhs->kind).tag<<" "<<(rhs->kind).data.integer.value<<" "<<(rhs->kind).tag<<endl;
  // 按照operator分类
  switch (binary.op)
  {
  case KOOPA_RBO_ADD: 

    break;
  
  case KOOPA_RBO_SUB:
    
    break;
  
  default:
    break;
  }

}

void Visit(const koopa_raw_integer_t &integer, char * RiscV)
{
  // not implied yet;
  
}

// 访问value:指令或者是值
void Visit(const koopa_raw_value_t &value, char * RiscV)
{
  // 根据指令类型判断后续需要如何访问, 这里得到的kind是一个tag和data的struct
  const auto &kind = value->kind;
  switch (kind.tag)
  {
    case KOOPA_RVT_RETURN:
      // 访问 return 指令
      Visit(kind.data.ret,RiscV);
      break;
    case KOOPA_RVT_INTEGER:
      // 访问 integer 指令
      Visit(kind.data.integer,RiscV);
      break;
    case KOOPA_RVT_BINARY:
      Visit(kind.data.binary,RiscV);
      
      break;
    default:
      // 其他类型暂时遇不到
      assert(false);
  }
}


// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb, char * RiscV)
{
  // 执行一些其他的必要操作
  // ...
  // 访问所有指令
  Visit(bb->insts,RiscV);// slice
}

// 访问函数
void Visit(const koopa_raw_function_t &func, char * RiscV)
{
  // 执行一些其他的必要操作
  // std::cout<<(func->name+1)<<":\n";
  strcat(RiscV, (func->name+1));
  strcat(RiscV, ":\n");
  // 访问所有基本块
  Visit(func->bbs,RiscV); //slice
}


// 这里的slice可以是各种level的代码片段, 从func到value, 然后对应不同的level调用不同的函数，这里是不同level进入更低level的借口，统一规约为slice然遍历执行
void Visit(const koopa_raw_slice_t &slice, char * RiscV)
{
  for (size_t i = 0; i < slice.len; ++i) 
  {
    auto ptr = slice.buffer[i]; // 此时buffer中的元素是void*
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) 
    {
      case KOOPA_RSIK_FUNCTION:
        // 访问函数
        Visit(reinterpret_cast<koopa_raw_function_t>(ptr),RiscV);
        break;
      case KOOPA_RSIK_BASIC_BLOCK:
        // 访问基本块
        Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr),RiscV);
        break;
      case KOOPA_RSIK_VALUE:
        // 访问指令
        Visit(reinterpret_cast<koopa_raw_value_t>(ptr),RiscV);
        break;
      default:
        // 我们暂时不会遇到其他内容, 于是不对其做任何处理
        assert(false);
    }
  }
}

// program 也就是程序的开始，递归的将一个koopa IR处理为risc-v，生成的方式就是遍历数据结构生成字符串
void Visit(const koopa_raw_program_t &program, char * RiscV)
{
  // 执行一些其他的必要操作
  // std::cout<<"  .text\n";
  // std::cout<<"  .globl main\n";
  strcat(RiscV, "  .text\n");
  strcat(RiscV, "  .globl main\n");


  // 访问所有全局变量
  Visit(program.values,RiscV);//slice
  // 访问所有函数
  Visit(program.funcs,RiscV);//slice
}

// 先从文本Koopa IR 生成一个 内存Koopa IR，然后处理对应Koopa IR生成 risc-v
void KoopaIR_2_RiscV(const char * IR, char * RiscV)
{
  // 解析字符串 str, 得到 Koopa IR 程序
  // 如果出现错误返回error code, 否则放回success code并将解析好的Koopa IR程序包存在program之中
  koopa_program_t program;
  koopa_error_code_t ret = koopa_parse_from_string(IR, &program);
  assert(ret == KOOPA_EC_SUCCESS);  // 确保解析时没有出错
  // 创建一个 raw program builder, 用来构建 raw program, 指针类型，需要delete，这里的builder 同样是一个void*
  koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
  // 将 Koopa IR 程序转换为 raw program, row praogram是一个包含两个row slices的struct, 分别是values和funcs，也就是最上层
  koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
  // 释放 Koopa IR 程序占用的内存，接下来的所有操作我们用raw就可以
  koopa_delete_program(program);
  
  Visit(raw,RiscV); //koopa_raw_program_t
    

  // 处理完成, 释放 raw program builder 占用的内存
  // 注意, raw program 中所有的指针指向的内存均为 raw program builder 的内存
  // 所以不要在 raw program 处理完毕之前释放 builder
  koopa_delete_raw_program_builder(builder);//program
   
}