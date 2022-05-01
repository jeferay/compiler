#pragma once
#include<iostream>
#include <memory>
#include <cstring>
#include <string>
using namespace std;

// 所有 AST 的基类，我们可以理解为一个节点，可以是叶节点或者内部节点，如果是叶节点表示终结符
class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual void  Dump(char * AST) const = 0;
  virtual void  Dump_IR(char * IR) const = 0;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
 public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> func_def;

  void Dump(char * AST) const override 
  {
    // std::cout << "CompUnitAST { ";
    strcat(AST,"CompUnitAST { ");
    func_def->Dump(AST);
    // std::cout << " }";
    strcat(AST," }");
    
  }

  void Dump_IR(char * IR) const override{
    func_def->Dump_IR(IR);
  }

};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident; // 这里没写构造函数，是后来赋值的
  std::unique_ptr<BaseAST> block;

  void Dump(char * AST) const override {
    // std::cout << "FuncDefAST { ";
    strcat(AST,"FuncDefAST { ");
    func_type->Dump(AST);
    
    // std::cout << ", " << ident << ", ";
    strcat(AST,", ");
    char *ident_str = const_cast<char *>(ident.c_str());
    strcat(AST,ident_str);
    strcat(AST,", ");
    
    block->Dump(AST);

    // std::cout << " }";
    strcat(AST," }");
  }

  void Dump_IR(char * IR) const override
  {
    // std::cout << "fun "<<"@"<<ident<<"(): ";

    strcat(IR,"fun @");
    strcat(IR,const_cast<char *>(ident.c_str()));
    strcat(IR,"(): ");

    func_type->Dump_IR(IR);
    block->Dump_IR(IR);
  }
};

// FunType 也是 BaseAST
class FuncTypeAST :public BaseAST
{
  public:
  std::string type;
  FuncTypeAST(string _type):type(_type){}
  FuncTypeAST(){} // 也定义一个空的构造函数

  void Dump(char * AST) const override
  {
    // std::cout << "FuncTypeAST { "<<type<<" }";
    strcat(AST,"FuncTypeAST { ");
    strcat(AST,const_cast<char *>(type.c_str()));
    strcat(AST," }");
  }

  void Dump_IR(char * IR) const override
  {
    if (type=="int")
    {
      // std::cout << "i32 ";
      strcat(IR,"i32 ");
    }
  }

};

// BlockAST 也是 BaseAST
class BlockAST: public BaseAST
{
  public:
  std::unique_ptr<BaseAST> stmt;
  void Dump(char * AST) const override
  {
    // std::cout << "BlockAST { ";
    strcat(AST,"BlockAST { ");
    stmt->Dump(AST);
    // std::cout << " }";
    strcat(AST," }");
  }

  void Dump_IR(char * IR) const override
  {
    // std::cout << "{\n";
    strcat(IR,"{\n");
    stmt->Dump_IR(IR);
    // std::cout << "}";
    strcat(IR,"}");
  }
};

// StmtAST 也是 BaseAST
class StmtAST: public BaseAST
{
  public:
  int number;
  StmtAST(int _number):number(_number){}
  StmtAST(){}

  void Dump(char * AST) const override
  {
    // std::cout << "StmtAST { "<<number<<" }";
    strcat(AST,"StmtAST { ");
    strcat(AST,const_cast<char *>(to_string(number).c_str()));
    strcat(AST," }");

  }

  void Dump_IR(char * IR) const override
  {
    // std::cout << "\%entry:\n  ret "<<to_string(number)<<"\n";
    strcat(IR,"\%entry:\n  ret ");
    strcat(IR,const_cast<char *>(to_string(number).c_str()));
    strcat(IR,"\n");
  }
};



// fun @main(): i32 {
// %entry:
//   ret 0
// }

