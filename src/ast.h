#pragma once
#include<iostream>
#include <memory>
#include <cstring>
#include <string>
using namespace std;


#define Integer 0
#define Register 1

// 新建一个用于IR指令返回值
typedef struct IR_Ins_Value
{
  int return_type;
  int return_value;
  IR_Ins_Value():return_type(-1),return_value(0){}
  //重载一下等于号
  IR_Ins_Value& operator=(const IR_Ins_Value &a)
  {
    return_type = a.return_type;
    return_value = a.return_value;
    return *this;
  }

  string get_IR_value(int bias_value = 0) const
  {
    if (return_type == Integer){
      return to_string(return_value);
    }
    else if (return_type == Register){
      return "%"+to_string(return_value + bias_value);
    }
    else return "not implied yet";
  }

  friend ostream & operator<<(ostream &out, IR_Ins_Value &A){
    out <<"IRV type "<<A.return_type <<" IRV value "<<A.return_value <<"\n";
    return out;
  }
  
}IR_Ins_Value;

// 所有 AST 的基类，我们可以理解为一个节点，可以是叶节点或者内部节点，如果是叶节点表示终结符
class BaseAST {
 public:
  int flag;
  IR_Ins_Value IRV;

  BaseAST():flag(-1){}
  virtual ~BaseAST(){};
  virtual void  Dump(char * AST) const {};
  virtual void  Dump_IR(char * IR) const {};
  virtual void  Set_IRV(int start_point){};// 有个start point的参数作为第一次分配寄存器的开始列表
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
 public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> func_def;

  void Dump(char * AST) const override 
  {
    strcat(AST,"CompUnitAST {\n");
    func_def->Dump(AST);
    strcat(AST,"}");
    
  }
  void Set_IRV(int start_point) override{
    func_def->Set_IRV(start_point);
    
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
    strcat(AST,"FuncDefAST {\n");
    func_type->Dump(AST);
    
    strcat(AST,",\n");
    char *ident_str = const_cast<char *>(ident.c_str());
    strcat(AST,ident_str);
    strcat(AST,",\n");
    
    block->Dump(AST);

    strcat(AST," }");
  }

  void Set_IRV(int start_point) override{
    block->Set_IRV(start_point);
  }
  void Dump_IR(char * IR) const override
  {
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

  void Set_IRV(int start_point) override{
    stmt->Set_IRV(start_point);
  }
  void Dump_IR(char * IR) const override
  {
    strcat(IR,"{\n\%entry:\n");
    stmt->Dump_IR(IR);
    strcat(IR,"}\n");
  }

};




// StmtAST 也是 BaseAST
class StmtAST: public BaseAST
{
  public:
  std::unique_ptr<BaseAST> exp;

  void Dump(char * AST) const override
  {
    strcat(AST,"StmtAST {\n");
    exp->Dump(AST);
    strcat(AST," }");

  }
  
  void Set_IRV(int start_point) override
  {
    if (IRV.return_type!=-1) return;
    if (flag==0)
    {
      exp->Set_IRV(start_point);
      IRV = exp->IRV;
    }

  }
  void Dump_IR(char * IR) const override
  {
    if (flag==0)
    {
      exp->Dump_IR(IR);
      strcat(IR,"  ret ");
      if (this->IRV.return_type==Integer){
        strcat(IR,const_cast<char *>(to_string(IRV.return_value).c_str())); 
        strcat(IR,"\n");
      }
      else if (IRV.return_type == Register){
        strcat(IR,const_cast<char*>(("%"+to_string(IRV.return_value)+"\n").c_str()));
      }

    }
  }
};

// Exp ::= LOrExp
class ExpAST: public BaseAST
{
  public:
  std::unique_ptr<BaseAST> lorexp;

  void Dump(char * AST) const override
  {
    if (flag==0){
      strcat(AST,"LOrExpAST {\n");
      lorexp->Dump(AST);
      strcat(AST,"}");
    }
  }
  void Set_IRV(int start_point) override
  {
    if (IRV.return_type!=-1) return;
    if (flag==0){
      lorexp->Set_IRV(start_point);
      IRV = lorexp->IRV;
    }

  }
  void Dump_IR(char * IR) const override
  {
    if (flag == 0){
      lorexp->Dump_IR(IR);
    }
  }
};

// UnaryExp ::= PrimaryExp | UnaryOp UnaryExp; done
class UnaryExpAST: public BaseAST
{
  public:
  
  // flag==0
  std::unique_ptr<BaseAST> primaryexp;

  //flag==1
  std::unique_ptr<BaseAST> unaryop;
  std::unique_ptr<BaseAST> unaryexp;

  void Dump(char * AST) const override
  {
    strcat(AST,"UnaryExpAST{\n");
    if (flag==0)
    {
      primaryexp->Dump(AST);
    }
    else if (flag==1)
    {
      unaryop->Dump(AST);
      strcat(AST,",\n");
      unaryexp->Dump(AST);
    }
    strcat(AST,"}");
  }

  void Set_IRV(int start_point) override
  {
    if (IRV.return_type!=-1) return;
    if (flag==0){
      primaryexp->Set_IRV(start_point); // 下层一定要先递归set之后才轮到下层
      IRV = primaryexp->IRV;
    }
    else if (flag==1){ // 要分情况讨论分解为 unaryop unaryexp的时候，op和exp分别的状态
      unaryexp->Set_IRV(start_point);

      if (unaryop->flag==0){ // "+"保持不变，不做这一步操作
        IRV = unaryexp->IRV;
      }
      else if (unaryop->flag == 1||unaryop->flag == 2){ // 分类讨论上层的type
        if (unaryexp->IRV.return_type==Integer){
          IRV.return_type = Register;
          IRV.return_value = start_point; //作为当前的第一个，从start point开始
        }
        else if (unaryexp->IRV.return_type==Register){
          IRV.return_type = Register;
          IRV.return_value = unaryexp->IRV.return_value + 1;// 从下一个寄存器开始, 因为不是从当前起始点开始
        }
      }
    }

  }
  
  void Dump_IR(char * IR) const override
  {
    // Set_IRV(); // 总是要先set irv
    if (flag==0){
      primaryexp->Dump_IR(IR);
    }
    else if (flag==1){
      unaryexp->Dump_IR(IR);
      // "+" 不处理
      if (unaryop->flag==0){} 
      else {
        string temp_IR = "  ";
        if (IRV.return_type==Register) temp_IR+= ("%"+to_string(IRV.return_value));
        temp_IR += " = ";
        // "-" 分类处理
        if (unaryop->flag==1){ 
          temp_IR+="sub 0, ";
          if (unaryexp->IRV.return_type==Integer){
            temp_IR+=(to_string(unaryexp->IRV.return_value) + "\n");
          }
          else if (unaryexp->IRV.return_type==Register){
            temp_IR+= ("%" + to_string(unaryexp->IRV.return_value) + "\n");
          }
        }
        // "!" 单独处理
        else if (unaryop->flag==2){
          temp_IR+="eq ";
          if (unaryexp->IRV.return_type==Integer){
            temp_IR+=to_string(unaryexp->IRV.return_value);
          }
          else if (unaryexp->IRV.return_type==Register){
            temp_IR+= ("%" + to_string(unaryexp->IRV.return_value));
          }
          temp_IR+=", 0\n";
          
        }
        strcat(IR,const_cast<char *>(temp_IR.c_str()));
      }
    }
    
  }
  
};

// PrimaryExp ::= "(" Exp ")" | Number; done
class PrimaryExpAST: public BaseAST
{
  public:
  
  // flag == 0
  std::unique_ptr<BaseAST> exp;
  //flag == 1
  int number;

  void Dump(char * AST) const override
  {
    strcat(AST,"PrimaryExpAST{\n");
    if (flag==0)
    {
      exp->Dump(AST);
    }
    else if (flag==1)
    {
      strcat(AST,const_cast<char *>(to_string(number).c_str())); 
    }
    strcat(AST,"}");

  }

  void Set_IRV(int start_point) override
  {
    if (IRV.return_type !=-1) return;
    if (flag==0){
      exp->Set_IRV(start_point);
      IRV = exp->IRV; // 保持一致
    }
    else if (flag==1)
    {
      IRV.return_type = Integer;
      IRV.return_value = number;
    }
  }
  void Dump_IR(char * IR) const override
  {
    // Set_IRV(); // 结构已经完全推断，可以直接set
    if (flag==0)
    {
      exp->Dump_IR(IR);
    }
  }
};

// AddExp ::= MulExp | AddExp AddOp MulExp; not yet
class AddExpAST: public BaseAST
{
  public:
  std::unique_ptr<BaseAST> addexp;
  std::unique_ptr<BaseAST> addop;
  std::unique_ptr<BaseAST> mulexp;

  void Set_IRV(int start_point) override 
  {
    if (IRV.return_type!=-1) return;
    if (flag==0){
      mulexp->Set_IRV(start_point);
      IRV = mulexp->IRV;
    }
    else if (flag==1){
      mulexp->Set_IRV(start_point);
      if (mulexp->IRV.return_type==Register){
        start_point = mulexp->IRV.return_value + 1; // 如果前半部分用到寄存器，则从下一个开始
      } 
      // 计算顺序是先乘法再加法
      addexp->Set_IRV(start_point);
      if (addexp->IRV.return_type==Register){
        start_point = addexp->IRV.return_value + 1; // 这里同理
      }
      IRV.return_type = Register;
      IRV.return_value = start_point;
    }
    

  }

  void Dump_IR(char * IR) const override{
    if (flag==0){
      mulexp->Dump_IR(IR);
    }
    else if (flag==1){
      mulexp->Dump_IR(IR);
      addexp->Dump_IR(IR);
      string temp_IR = "  "+IRV.get_IR_value()+" = ";
      if (addop->flag==0){
        temp_IR+=("add ");
      }
      else if (addop->flag==1){
        temp_IR+=("sub ");
      }
      temp_IR+=(addexp->IRV.get_IR_value() + ", " + mulexp->IRV.get_IR_value() + "\n");
      strcat(IR,const_cast<char *>(temp_IR.c_str())); 
    }

  }
};

// MulExp ::=UnaryExp | MulExp MulOp UnaryExp
class MulExpAST: public BaseAST
{
  public:
  std::unique_ptr<BaseAST> mulexp;
  std::unique_ptr<BaseAST> mulop;
  std::unique_ptr<BaseAST> unaryexp;

  void Set_IRV(int start_point) override{
    if (IRV.return_type!=-1)return;
    if (flag==0){
      unaryexp->Set_IRV(start_point);
      IRV = unaryexp->IRV;
    }
    else if(flag==1){
      // 先计算unaryexp 再计算乘法
      unaryexp->Set_IRV(start_point);
      if (unaryexp->IRV.return_type==Register){
        start_point = unaryexp->IRV.return_value + 1;
      }
      mulexp->Set_IRV(start_point);
      if (mulexp->IRV.return_type==Register){
        start_point = mulexp->IRV.return_value + 1;
      }
      IRV.return_type = Register;
      IRV.return_value = start_point;
    }
  }
  void Dump_IR(char * IR) const override{
    if (flag==0){
      unaryexp->Dump_IR(IR);
    }
    else if(flag==1){
      unaryexp->Dump_IR(IR); //先unaryexp计算
      mulexp->Dump_IR(IR);
      string temp_IR = "  " + IRV.get_IR_value() + " = ";
      switch(mulop->flag){
        case 0: temp_IR+="mul ";break;
        case 1: temp_IR+="div ";break;
        case 2: temp_IR+="mod ";break;
      }
      temp_IR+=(mulexp->IRV.get_IR_value()+", "+unaryexp->IRV.get_IR_value()+"\n");
      strcat(IR,const_cast<char *>(temp_IR.c_str())); 
    }
  }
};

// RelExp ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
class RelExpAST: public BaseAST
{
  public:
  std::unique_ptr<BaseAST> addexp;
  std::unique_ptr<BaseAST> relop;
  std::unique_ptr<BaseAST> relexp;

  void Set_IRV(int start_point) override{
    if (IRV.return_type!=-1)return;
    if (flag==0){
      addexp->Set_IRV(start_point);
      IRV = addexp->IRV;
    }
    else if (flag==1){
      addexp->Set_IRV(start_point);
      if (addexp->IRV.return_type==Register){
        start_point = addexp->IRV.return_value + 1;
      }
      relexp->Set_IRV(start_point);
      if (relexp->IRV.return_type==Register){
        start_point = relexp->IRV.return_value + 1;
      }
      IRV.return_type = Register;
      IRV.return_value = start_point;
    }

  }

  void Dump_IR(char * IR) const override{
    if (flag==0){
      addexp->Dump_IR(IR);
    }
    else if (flag==1){
      addexp->Dump_IR(IR);
      relexp->Dump_IR(IR);
      string temp_IR = "  " + IRV.get_IR_value() + " = ";
      switch (relop->flag)
      {
        case 0: temp_IR+="lt ";break;
        case 1: temp_IR+="gt ";break;
        case 2: temp_IR+="le ";break;
        case 3: temp_IR+="ge ";break;
      }
      temp_IR+=(relexp->IRV.get_IR_value()+", "+addexp->IRV.get_IR_value()+"\n");
      strcat(IR,const_cast<char *>(temp_IR.c_str())); 
    }

  }
  

};

// EqExp ::= RelExp | EqExp ("==" | "!=") RelExp;
class EqExpAST: public BaseAST
{
  public:
  std::unique_ptr<BaseAST> eqexp;
  std::unique_ptr<BaseAST> eqop;
  std::unique_ptr<BaseAST> relexp;

  void Set_IRV(int start_point) override{
    if (IRV.return_type!=-1)return;
    if (flag==0){
      relexp->Set_IRV(start_point);
      IRV = relexp->IRV;
    }
    else if (flag==1){
      relexp->Set_IRV(start_point);
      if (relexp->IRV.return_type==Register){
        start_point = relexp->IRV.return_value + 1;
      }
      eqexp->Set_IRV(start_point);
      if (eqexp->IRV.return_type==Register){
        start_point = eqexp->IRV.return_value + 1;
      }
      IRV.return_type = Register;
      IRV.return_value = start_point;
    }
  }

  void Dump_IR(char * IR) const override{
    if (flag==0){
      relexp->Dump_IR(IR);
    }
    else if (flag==1){
      relexp->Dump_IR(IR);
      eqexp->Dump_IR(IR);
      string temp_IR = "  " + IRV.get_IR_value() + " = ";
      switch (eqop->flag)
      {
        case 0: temp_IR+="eq ";break;
        case 1: temp_IR+="ne ";break;
      }
      temp_IR+=(eqexp->IRV.get_IR_value()+", "+relexp->IRV.get_IR_value()+"\n");
      strcat(IR,const_cast<char *>(temp_IR.c_str())); 
    }
    
  }

};

// LAndExp ::= EqExp | LAndExp "&&" EqExp;
class LAndExpAST: public BaseAST
{
  public:
  std::unique_ptr<BaseAST> eqexp;
  std::unique_ptr<BaseAST> landexp;
  void Set_IRV(int start_point) override{
    if (IRV.return_type!=-1)return;
    if (flag==0){
      eqexp->Set_IRV(start_point);
      IRV = eqexp->IRV;
    }
    else if (flag==1){
      eqexp->Set_IRV(start_point);
      if (eqexp->IRV.return_type==Register){
        start_point = eqexp->IRV.return_value + 1;
      }
      landexp->Set_IRV(start_point);
      if (landexp->IRV.return_type==Register){
        start_point = landexp->IRV.return_value + 1;
      }
      IRV.return_type = Register;
      IRV.return_value = start_point + 2; //逻辑land拆解为三部，先两边分别和0取neq再and
    }

  }

  void Dump_IR(char * IR) const override{
    if (flag==0){
      eqexp->Dump_IR(IR);
    }
    else if (flag==1){
      eqexp->Dump_IR(IR);
      landexp->Dump_IR(IR);
      string temp_IR = "  " + IRV.get_IR_value(-2) + " = ne "+landexp->IRV.get_IR_value() + ", 0\n";
      temp_IR += "  " + IRV.get_IR_value(-1) + " = ne "+ eqexp->IRV.get_IR_value() + ", 0\n";
      temp_IR += "  " + IRV.get_IR_value(0) + " = and " + IRV.get_IR_value(-2) + ", " + IRV.get_IR_value(-1) + "\n";
      strcat(IR,const_cast<char *>(temp_IR.c_str())); 
    }
    
  }

};

// LOrExp ::= LAndExp | LOrExp "||" LAndExp;
class LOrExpAST: public BaseAST
{
  public:
  std::unique_ptr<BaseAST> landexp;
  std::unique_ptr<BaseAST> lorexp;

  void Set_IRV(int start_point) override{
    if (IRV.return_type!=-1)return;
    if (flag==0){
      landexp->Set_IRV(start_point);
      IRV = landexp->IRV;
    }

    else if (flag==1){
      landexp->Set_IRV(start_point);
      if (landexp->IRV.return_type==Register){
        start_point = landexp->IRV.return_value + 1;
      }
      lorexp->Set_IRV(start_point);
      if (lorexp->IRV.return_type==Register){
        start_point = lorexp->IRV.return_value + 1;
      }
      IRV.return_type = Register;
      IRV.return_value = start_point + 1;// 用位运算拼凑
    }
  }

  void Dump_IR(char * IR) const override{
    if (flag==0){
      landexp->Dump_IR(IR);
    }
    else if (flag==1){
      landexp->Dump_IR(IR);
      lorexp->Dump_IR(IR);
      string temp_IR = "  " + IRV.get_IR_value(-1) + " = ";
      temp_IR += "or ";
      temp_IR += (lorexp->IRV.get_IR_value() + ", " + landexp->IRV.get_IR_value() + "\n");
      temp_IR += ("  " + IRV.get_IR_value(0) + " = ");
      temp_IR += ("ne " + IRV.get_IR_value(-1) + ", 0\n");
      strcat(IR,const_cast<char *>(temp_IR.c_str())); 
    }
    
  }
  
};


// UnaryOp ::= "+" | "-" | "!"; done
class UnaryOpAST: public BaseAST
{
  public:
  void Dump(char * AST) const override
  {
    strcat(AST,"UnaryOpAST{ ");
    switch (flag)
    {
    case 0: strcat(AST,"+");break;
    case 1: strcat(AST,"-");break;
    case 2: strcat(AST,"!");break;
    }
    strcat(AST," }\n");
  }
  void Dump_IR(char * IR) const override{}
};


// AddOp ::= "+" | "-"
class AddOpAST: public BaseAST
{
  public:
  void Dump(char * AST) const override{}
  void Dump_IR(char * IR) const override{}
};

// MulOp ::= "*" | "/" | "%"
class MulOpAST: public BaseAST
{
  public:
  void Dump(char * AST) const override{}
  void Dump_IR(char * IR) const override{}
};
// ("<" | ">" | "<=" | ">=")
class RelOpAST: public BaseAST
{
  public:
  void Dump(char * AST) const override{}
  void Dump_IR(char * IR) const override{}

};

//("==" | "!=")
class EqOpAST: public BaseAST
{
  public:
  void Dump(char * AST) const override{}
  void Dump_IR(char * IR) const override{}

};

