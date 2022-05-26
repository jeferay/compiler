#ifndef _ast_H_
#define _ast_H_
#endif

#pragma once
#include<iostream>
#include <memory>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <assert.h>
using namespace std;

#define Integer 0
#define Register 1

#define ConstVar 0 
#define Var 1

// 新建一个用于IR指令返回值
typedef struct IR_Ins_Value
{
	int return_type;
	int return_value;
	IR_Ins_Value() :return_type(-1), return_value(0) {}
	//重载一下等于号
	IR_Ins_Value& operator=(const IR_Ins_Value& a)
	{
		return_type = a.return_type;
		return_value = a.return_value;
		return *this;
	}

	std::string get_IR_value(int bias_value = 0) const
	{
		if (return_type == Integer) {
			return std::to_string(return_value);
		}
		else if (return_type == Register) {
			return "%" + std::to_string(return_value + bias_value);
		}
		else return "not implied yet";
	}

	friend std::ostream& operator<<(std::ostream& out, IR_Ins_Value& A) {
		out << "IRV type " << A.return_type << " IRV value " << A.return_value << "\n";
		return out;
	}

}IR_Ins_Value;

typedef struct Varient {
	int tag;
	int value;
	Varient() :tag(-1), value(0) {}
	friend std::ostream& operator<<(std::ostream& out, Varient& st) {
		out << "tag" << st.tag << "value" << st.value << endl;
    return out;
	}
	Varient(int _tag, int _value) :tag(_tag), value(_value) {}
}Varient;

typedef struct SymbolTable {
public:
	std::map<std::string, Varient> pair_map;
	SymbolTable() {
		pair_map.clear();
	}
	void insert(std::string key, int tag, int value) {
		pair_map.insert(std::pair<std::string, Varient>(key, Varient(tag, value)));
	}
	void insert(std::string key, Varient v) {
		pair_map.insert(std::pair<std::string, Varient>(key, v));
	}
	Varient find(std::string key) {
		std::map<std::string, Varient> ::iterator l_it = pair_map.find(key);
		return l_it->second;
	}
	friend std::ostream& operator<<(std::ostream& out, SymbolTable& st) {
		for (auto iter = st.pair_map.begin(); iter != st.pair_map.end(); iter++) {
			out << iter->first << " " << iter->second << "\n";
		}
		return out;
	}
}SymbolTable;


// 所有 AST 的基类，我们可以理解为一个节点，可以是叶节点或者内部节点，如果是叶节点表示终结符
class BaseAST {
public:
	int flag;
	IR_Ins_Value IRV;

	BaseAST() :flag(-1) {}
	virtual ~BaseAST() {};

	virtual void  Dump_IR(char* IR) const {};
	virtual void  Set_IRV(int start_point) {};// 有个start point的参数作为第一次分配寄存器的开始列表
	virtual int calculate() { assert(false); return 0; };
	virtual void set_symbol_table() {};
	virtual Varient lookup_table() { assert(false); return Varient(); };
};

class CompUnitAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> func_def;
	virtual void Set_IRV(int start_point) override;
	virtual void Dump_IR(char* IR) const override;
	virtual void set_symbol_table() override;
	void output_symbol_table();
};

class FuncDefAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> func_type;
	std::string ident; // 这里没写构造函数，是后来赋值的
	std::unique_ptr<BaseAST> block;
	virtual void Set_IRV(int start_point) override;
	virtual void set_symbol_table() override;
	virtual void Dump_IR(char* IR) const override;
};

// BlockAST::='{' '}'|'{' BlockItemVec '}' //not yet
class BlockAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> blockitemvec;
	virtual void Set_IRV(int start_point) override;
	virtual void set_symbol_table() override;
	virtual void Dump_IR(char* IR) const override;
};

class FuncTypeAST :public BaseAST
{
public:
	std::string type;
	FuncTypeAST(std::string _type);
	FuncTypeAST();
	virtual void Dump_IR(char* IR) const;
};



//BlockItemVec::=BlockItemVec BlockItem | BlockItem
class BlockItemVecAST : public BaseAST {
public:
	std::vector<unique_ptr<BaseAST>> itemvec;
	virtual void set_symbol_table() override;
	virtual void Dump_IR(char* IR) const override;
	virtual void Set_IRV(int start_point) override;
};

// BlockItem::=Decl|Stmt
class BlockItemAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> decl;
	std::unique_ptr<BaseAST> stmt;
	virtual void Dump_IR(char* IR) const override;
	virtual void set_symbol_table() override;
	virtual void Set_IRV(int start_point) override;
};

// Decl::=ConstDecl|VarDecl
class DeclAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> constdecl;
	std::unique_ptr<BaseAST> vardecl;
	virtual void Dump_IR(char* IR) const override;
	virtual void set_symbol_table() override;
	virtual void Set_IRV(int start_point) override;

};

// COnstDecl::="const" BType ConstDefVec ';' const的定义不需要irv设定或者dumpir
class ConstDeclAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> btype;
	std::unique_ptr<BaseAST> constdefvec;     
	virtual void set_symbol_table() override;
};

//Btype::="int"
class BtypeAST : public BaseAST {
};


// ConstDefVec::= ConstDefVec ',' ConstDef | ConstDef
class ConstDefVecAST : public BaseAST {
public:
	vector<unique_ptr<BaseAST>> itemvec;
	virtual void set_symbol_table() override;

};

// ConstDefAST::= IDENT '=' ConstInitVal
class ConstDefAST : public BaseAST {
	// 在constdef的时候把ident加入到符号表之中
public:
	std::string ident;
	std::unique_ptr<BaseAST> constinitval;
	int calculate() override; // 每个constdefast开始可能用到calculate，保证一定是常量计算
	void set_symbol_table() override;

};

// ConstInitVal::= ConstExp
class ConstInitValAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> constexp;
	int calculate() override;
};

// LVal::= IDENT
class LValAST : public BaseAST {
public:
	std::string ident;
	virtual Varient lookup_table()override;
	virtual void Set_IRV(int start_point)override;
};

// ConstExp ::= Exp
class ConstExpAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> exp;
	int calculate() override;
};

// VarDecl::=Btype VarDefVec ';'
class VarDeclAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> btype;
	std::unique_ptr<BaseAST> vardefvec;
	virtual void Set_IRV(int start_point) override;
	virtual void Dump_IR(char* IR) const override;
	virtual void set_symbol_table() override;
};

//VarDefVec::=VarDefVec ',' VarDef | VarDef
class VarDefVecAST : public BaseAST {
public:
	std::vector<std::unique_ptr<BaseAST>> itemvec;
	virtual void Set_IRV(int start_point) override;
	virtual void Dump_IR(char* IR) const override;
	virtual void set_symbol_table() override;

};

// VarDef::=IDENT|IDENT '=' InitVal
class VarDefAST : public BaseAST {
public:
	std::string ident;
	std::unique_ptr<BaseAST> initval;
	virtual void Set_IRV(int start_point) override;
	virtual void Dump_IR(char* IR) const override;
	virtual void set_symbol_table() override;
};

// InitVal::=Exp
class InitValAST :public BaseAST {
public:
	std::unique_ptr<BaseAST> exp;
	virtual void Dump_IR(char* IR) const override;
	virtual void Set_IRV(int start_point) override;
};


// StmtAST::= RETURN Exp ';'| Lval '=' Exp ';'
class StmtAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> exp;
	std::unique_ptr<BaseAST> lval;
	void Set_IRV(int start_point) override;
	void Dump_IR(char* IR) const override;
};

// Exp ::= LOrExp
class ExpAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> lorexp;
	int calculate() override;
	void Set_IRV(int start_point) override;
	void Dump_IR(char* IR) const override;
};

// UnaryExp ::= PrimaryExp | UnaryOp UnaryExp
class UnaryExpAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> primaryexp;
	std::unique_ptr<BaseAST> unaryop;
	std::unique_ptr<BaseAST> unaryexp;
	int calculate() override;
	void Set_IRV(int start_point) override;
	void Dump_IR(char* IR) const override;
};

// PrimaryExp ::= "(" Exp ")" | Number| LVal not yet
class PrimaryExpAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> exp;
	int number;
	std::unique_ptr<BaseAST> lval;
	int calculate() override;
	void Set_IRV(int start_point) override;
	void Dump_IR(char* IR) const override;
};

// AddExp ::= MulExp | AddExp AddOp MulExp;
class AddExpAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> addexp;
	std::unique_ptr<BaseAST> addop;
	std::unique_ptr<BaseAST> mulexp;

	int calculate() override;
	void Set_IRV(int start_point) override;
	void Dump_IR(char* IR) const override;
};

// MulExp ::=UnaryExp | MulExp MulOp UnaryExp
class MulExpAST : public BaseAST
{
public:
	std::unique_ptr<BaseAST> mulexp;
	std::unique_ptr<BaseAST> mulop;
	std::unique_ptr<BaseAST> unaryexp;
	int calculate() override;
	void Set_IRV(int start_point) override;
	void Dump_IR(char* IR) const override;
};

// RelExp ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
class RelExpAST : public BaseAST
{
public:
	std::unique_ptr<BaseAST> addexp;
	std::unique_ptr<BaseAST> relop;
	std::unique_ptr<BaseAST> relexp;
	int calculate() override;
	void Set_IRV(int start_point) override;
	void Dump_IR(char* IR) const override;
};

// EqExp ::= RelExp | EqExp ("==" | "!=") RelExp;
class EqExpAST : public BaseAST
{
public:
	std::unique_ptr<BaseAST> eqexp;
	std::unique_ptr<BaseAST> eqop;
	std::unique_ptr<BaseAST> relexp;

	int calculate() override;
	void Set_IRV(int start_point) override;
	void Dump_IR(char* IR) const override;
};

// LAndExp ::= EqExp | LAndExp "&&" EqExp;
class LAndExpAST : public BaseAST
{
public:
	std::unique_ptr<BaseAST> eqexp;
	std::unique_ptr<BaseAST> landexp;
	int calculate() override;
	void Set_IRV(int start_point) override;
	void Dump_IR(char* IR) const override;
};

// LOrExp ::= LAndExp | LOrExp "||" LAndExp;
class LOrExpAST : public BaseAST
{
public:
	std::unique_ptr<BaseAST> landexp;
	std::unique_ptr<BaseAST> lorexp;
	int calculate() override;
	void Set_IRV(int start_point) override;
	void Dump_IR(char* IR) const override;
};

// UnaryOp ::= "+" | "-" | "!";
class UnaryOpAST : public BaseAST {
public:
	void Dump_IR(char* IR) const override;
};

// AddOp ::= "+" | "-"
class AddOpAST : public BaseAST {
public:
	void Dump_IR(char* IR) const override;
};

// MulOp ::= "*" | "/" | "%"
class MulOpAST : public BaseAST {
public:
	void Dump_IR(char* IR) const override;
};
// ("<" | ">" | "<=" | ">=")
class RelOpAST : public BaseAST {
public:
	void Dump_IR(char* IR) const override;

};

//("==" | "!=")
class EqOpAST : public BaseAST {
public:
	void Dump_IR(char* IR) const override;
};

