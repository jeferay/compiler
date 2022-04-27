
#pragma once
#include<iostream>
#include <memory>
using namespace std;

// 所有 AST 的基类，我们可以理解为一个节点，可以是叶节点或者内部节点，如果是叶节点表示终结符
class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual void  Dump() const = 0;
  virtual void  Dump_IR() const = 0;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
 public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> func_def;

  void Dump() const override {
    std::cout << "CompUnitAST { ";
    func_def->Dump();
    std::cout << " }";
  }

  void Dump_IR() const override{
    func_def->Dump_IR();
  }

};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident; // 这里没写构造函数，是后来赋值的
  std::unique_ptr<BaseAST> block;

  void Dump() const override {
    std::cout << "FuncDefAST { ";
    func_type->Dump();
    std::cout << ", " << ident << ", ";
    block->Dump();
    std::cout << " }";
  }

  void Dump_IR() const override
  {
    std::cout << "fun "<<"@"<<ident<<"(): ";
    func_type->Dump_IR();
    block->Dump_IR();
  }
};

// FunType 也是 BaseAST
class FuncTypeAST :public BaseAST
{
  public:
  std::string type;
  FuncTypeAST(string _type):type(_type){}
  FuncTypeAST(){} // 也定义一个空的构造函数

  void Dump() const override
  {
    std::cout << "FuncTypeAST { ";
    std::cout << type;
    std::cout << " }";
  }

  void Dump_IR() const override
  {
    if (type=="int")
      std::cout << "i32 ";
  }

};

// BlockAST 也是 BaseAST
class BlockAST: public BaseAST
{
  public:
  std::unique_ptr<BaseAST> stmt;
  void Dump()const override
  {
    std::cout << "BlockAST { ";
    stmt->Dump();
    std::cout << " }";
  }

  void Dump_IR() const override
  {
    std::cout << "{"<<endl;
    stmt->Dump_IR();
    std::cout << "}";
  }
};

// StmtAST 也是 BaseAST
class StmtAST: public BaseAST{
  public:
  int number;
  StmtAST(int _number):number(_number){}
  StmtAST(){}

  void Dump() const override
  {
    std::cout << "StmtAST { ";
    std::cout << number;
    std::cout << " }";
  }

  void Dump_IR() const override
  {
    cout << "\%entry:\n"<<"  ";
    cout << "ret ";
    cout << to_string(number)<<endl;
  }
};

//CompUnitAST { FuncDefAST { FuncTypeAST { int }, main, BlockAST { StmtAST { 0 } } } }

// fun @main(): i32 {  // main 函数的定义
// %entry:             // 入口基本块
//   ret 0             // return 0
// }

// fun @main(): i32 {
// %entry:
//   ret 0
// }

