
#pragma once
#include<iostream>
#include <memory>
using namespace std;

// 所有 AST 的基类，我们可以理解为一个节点，可以是叶节点或者内部节点，如果是叶节点表示终结符
class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual void  Dump() const = 0;
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
};


