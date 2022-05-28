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
#include <algorithm>
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
	IR_Ins_Value& operator=(const IR_Ins_Value& a) {
		return_type = a.return_type;
		return_value = a.return_value;
		return *this;
	}

	std::string get_IR_value(int bias_value = 0) const {
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
	std::string name;
	int tag;
	int value;//如果tag是Const则value就是具体值，如果是Var就放置编号
	Varient() :name(" "), tag(-1), value(-1) {}
	friend std::ostream& operator<<(std::ostream& out, Varient& st) {
		out << "name " << st.name << " tag " << st.tag << " value" << st.value << endl;
		return out;
	}
	Varient(std::string _name, int _tag, int _value) :name(_name), tag(_tag), value(_value) {}
	Varient& operator=(const Varient& a) {
		name = a.name;
		tag = a.tag;
		value = a.value;
		return *this;
	}
	string get_str_value(int multi_def = false) {
		if (tag == ConstVar) {
			return std::to_string(value);
		}
		else if (tag == Var) {
			if (multi_def) return "@" + name + "_" + std::to_string(value);
			else return "@" + name;
		}
	}
}Varient;

// 每一个ast都包含一个对应层次的symboltable，里面仅存放当前层数的 not yet
typedef struct SymbolTable {
public:
	std::map<std::string, Varient> pair_map;
	std::shared_ptr<SymbolTable> pre;
	std::vector<shared_ptr<SymbolTable>>sons;
	SymbolTable() {
		pair_map.clear();
		pre = nullptr;
		sons.clear();
	}
	// 按照tag来insert
	void insert(std::string key, int tag, int value) {
		if (tag == ConstVar) {
			pair_map.insert(std::pair<std::string, Varient>(key, Varient(key, tag, value)));
		}
		else if (tag == Var) {
			value = search_latest_sub_tree(get_root(), key) + 1;
			pair_map.insert(std::pair<std::string, Varient>(key, Varient(key, tag, value)));
		}
	}

	Varient search_until_root(std::string key) {
		std::map<std::string, Varient> ::iterator l_it = pair_map.find(key);
		if (l_it != pair_map.end()) {
			return l_it->second;
		}
		else if (pre != nullptr) {
			return pre->search_until_root(key);
		}
		assert(false);//一定能搜到，语法正确的情况下
	}
	//找到根节点,返回引用
	SymbolTable& get_root() {
		if (pre == nullptr) {
			return *this;
		}
		return pre->get_root();
	}
	// 搜索子树中编号最大的key，只用于变量标号查询,如果没有则返回0
	int search_latest_sub_tree(SymbolTable& root, std::string key) {
		std::map<std::string, Varient> ::iterator l_it = root.pair_map.find(key);
		int ret_value = 0;
		if (l_it != root.pair_map.end() && l_it->second.tag == Var) { // 必须是变量才算，该函数只用于变量查询标号
			ret_value = max(ret_value, l_it->second.value);
		}
		for (int i = 0; i < sons.size(); ++i) {
			ret_value = max(ret_value, search_latest_sub_tree(*sons[i], key));
		}
		return ret_value;
	}

	int multi_def(std::string key) {
		return (search_latest_sub_tree(get_root(), key) > 1);
	}

	friend std::ostream& operator<<(std::ostream& out, SymbolTable& st) {
		for (auto iter = st.pair_map.begin(); iter != st.pair_map.end(); iter++) {
			out << "key " << iter->first << " " << iter->second << "\n";
		}
		return out;
	}

}SymbolTable;


// 所有 AST 的基类，我们可以理解为一个节点，可以是叶节点或者内部节点，如果是叶节点表示终结符
class BaseAST {
public:
	int flag;
	IR_Ins_Value IRV;
	std::shared_ptr<SymbolTable> symtable;
	BaseAST() :flag(-1), symtable(nullptr) {}
	virtual ~BaseAST() {};

	virtual void  Dump_IR(char* IR) const {};
	virtual void  Set_IRV(int start_point) {};// 有个start point的参数作为第一次分配寄存器的开始列表
	virtual int calculate() { assert(false); return 0; };
	virtual void set_symbol_table(std::shared_ptr<SymbolTable> now) {};

};

class CompUnitAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> func_def;
	CompUnitAST();
	virtual ~CompUnitAST() override;
	virtual void Set_IRV(int start_point) override;
	virtual void Dump_IR(char* IR) const override;
	void set_symbol_table(std::shared_ptr<SymbolTable> now) override;
	void output_symbol_table();
};

class FuncDefAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> func_type;
	std::string ident; // 这里没写构造函数，是后来赋值的
	std::unique_ptr<BaseAST> block;
	FuncDefAST();
	virtual ~FuncDefAST() override;
	virtual void Set_IRV(int start_point) override;
	virtual void set_symbol_table(std::shared_ptr<SymbolTable> now) override;
	virtual void Dump_IR(char* IR) const override;
};

// BlockAST::='{' '}'|'{' BlockItemVec '}' //not yet
class BlockAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> blockitemvec;
	BlockAST();
	virtual ~BlockAST() override;
	virtual void Set_IRV(int start_point) override;
	virtual void set_symbol_table(std::shared_ptr<SymbolTable> now) override;
	virtual void Dump_IR(char* IR) const override;
};

class FuncTypeAST :public BaseAST
{
public:
	std::string type;
	FuncTypeAST(std::string _type);
	FuncTypeAST();
	~FuncTypeAST() override;
	virtual void Dump_IR(char* IR) const;
};



//BlockItemVec::=BlockItemVec BlockItem | BlockItem
class BlockItemVecAST : public BaseAST {
public:
	std::vector<unique_ptr<BaseAST>> itemvec;
	BlockItemVecAST();
	virtual ~BlockItemVecAST() override;
	virtual void set_symbol_table(std::shared_ptr<SymbolTable> now) override;
	virtual void Dump_IR(char* IR) const override;
	virtual void Set_IRV(int start_point) override;
};

// BlockItem::=Decl|Stmt
class BlockItemAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> decl;
	std::unique_ptr<BaseAST> stmt;
	BlockItemAST();
	virtual~BlockItemAST() override;
	virtual void Dump_IR(char* IR) const override;
	virtual void set_symbol_table(std::shared_ptr<SymbolTable> now) override;
	virtual void Set_IRV(int start_point) override;
};

// Decl::=ConstDecl|VarDecl
class DeclAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> constdecl;
	std::unique_ptr<BaseAST> vardecl;
	DeclAST();
	virtual ~DeclAST()override;
	virtual void Dump_IR(char* IR) const override;
	virtual void set_symbol_table(std::shared_ptr<SymbolTable> now) override;
	virtual void Set_IRV(int start_point) override;

};

// COnstDecl::="const" BType ConstDefVec ';' const的定义不需要irv设定或者dumpir
class ConstDeclAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> btype;
	std::unique_ptr<BaseAST> constdefvec;
	ConstDeclAST();
	virtual ~ConstDeclAST()override;
	virtual void set_symbol_table(std::shared_ptr<SymbolTable> now) override;
};

//Btype::="int"
class BtypeAST : public BaseAST {
public:
	BtypeAST();
	virtual ~BtypeAST() override;
};


// ConstDefVec::= ConstDefVec ',' ConstDef | ConstDef
class ConstDefVecAST : public BaseAST {
public:
	vector<unique_ptr<BaseAST>> itemvec;
	ConstDefVecAST();
	virtual ~ConstDefVecAST() override;
	virtual void set_symbol_table(std::shared_ptr<SymbolTable> now) override;

};

// ConstDefAST::= IDENT '=' ConstInitVal
class ConstDefAST : public BaseAST {
	// 在constdef的时候把ident加入到符号表之中
public:
	std::string ident;
	std::unique_ptr<BaseAST> constinitval;
	ConstDefAST();
	virtual ~ConstDefAST() override;
	int calculate() override; // 每个constdefast开始可能用到calculate，保证一定是常量计算
	virtual void set_symbol_table(std::shared_ptr<SymbolTable> now) override;

};

// ConstInitVal::= ConstExp
class ConstInitValAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> constexp;
	ConstInitValAST();
	virtual ~ConstInitValAST() override;
	int calculate() override;
};

// LVal::= IDENT
class LValAST : public BaseAST {
public:
	std::string ident;
	LValAST();
	virtual  ~LValAST();
	virtual void Set_IRV(int start_point)override;
	virtual void set_symbol_table(std::shared_ptr<SymbolTable> now) override;
};

// ConstExp ::= Exp
class ConstExpAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> exp;
	ConstExpAST();
	virtual ~ConstExpAST() override;
	int calculate() override;
};

// VarDecl::=Btype VarDefVec ';'
class VarDeclAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> btype;
	std::unique_ptr<BaseAST> vardefvec;
	VarDeclAST();
	virtual  ~VarDeclAST() override;
	virtual void Set_IRV(int start_point) override;
	virtual void Dump_IR(char* IR) const override;
	virtual void set_symbol_table(std::shared_ptr<SymbolTable> now) override;
};

//VarDefVec::=VarDefVec ',' VarDef | VarDef
class VarDefVecAST : public BaseAST {
public:
	std::vector<std::unique_ptr<BaseAST>> itemvec;
	VarDefVecAST();
	virtual ~VarDefVecAST() override;
	virtual void Set_IRV(int start_point) override;
	virtual void Dump_IR(char* IR) const override;
	virtual void set_symbol_table(std::shared_ptr<SymbolTable> now) override;

};

// VarDef::=IDENT|IDENT '=' InitVal
class VarDefAST : public BaseAST {
public:
	std::string ident;
	std::unique_ptr<BaseAST> initval;
	VarDefAST();
	virtual  ~VarDefAST() override;
	virtual void Set_IRV(int start_point) override;
	virtual void Dump_IR(char* IR) const override;
	virtual void set_symbol_table(std::shared_ptr<SymbolTable> now) override;
};

// InitVal::=Exp
class InitValAST :public BaseAST {
public:
	std::unique_ptr<BaseAST> exp;
	InitValAST();
	virtual ~InitValAST() override;
	virtual void Dump_IR(char* IR) const override;
	virtual void Set_IRV(int start_point) override;
	virtual void set_symbol_table(std::shared_ptr<SymbolTable> now) override;
};


// StmtAST::=  Lval '=' Exp ';' | ExpExist ';' |Block | "return" ExpExist ';'
class StmtAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> exp;
	std::unique_ptr<BaseAST> lval;
	std::unique_ptr<BaseAST> expexist;
	std::unique_ptr<BaseAST> block;
	StmtAST();
	virtual  ~StmtAST();
	void Set_IRV(int start_point) override;
	void Dump_IR(char* IR) const override;
	virtual void set_symbol_table(std::shared_ptr<SymbolTable> now) override;

};

// ExpExist::Exp|ε
class ExpExistAST : public BaseAST {
public:
	std::unique_ptr<BaseAST>exp;
	ExpExistAST();
	virtual ~ExpExistAST()override;
	virtual void set_symbol_table(std::shared_ptr<SymbolTable> now) override;
	virtual void Set_IRV(int start_point) override;
	virtual void Dump_IR(char* IR) const override;
};

// Exp ::= LOrExp
class ExpAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> lorexp;
	ExpAST();
	virtual ~ExpAST();
	int calculate() override;
	virtual void set_symbol_table(std::shared_ptr<SymbolTable> now) override;
	void Set_IRV(int start_point) override;
	void Dump_IR(char* IR) const override;
};

// UnaryExp ::= PrimaryExp | UnaryOp UnaryExp
class UnaryExpAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> primaryexp;
	std::unique_ptr<BaseAST> unaryop;
	std::unique_ptr<BaseAST> unaryexp;
	UnaryExpAST();
	virtual  ~UnaryExpAST() override;
	int calculate() override;
	virtual void set_symbol_table(std::shared_ptr<SymbolTable> now) override;
	void Set_IRV(int start_point) override;
	void Dump_IR(char* IR) const override;
};

// PrimaryExp ::= "(" Exp ")" | Number| LVal not yet
class PrimaryExpAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> exp;
	int number;
	std::unique_ptr<BaseAST> lval;
	PrimaryExpAST();
	virtual  ~PrimaryExpAST()override;
	int calculate() override;
	virtual void set_symbol_table(std::shared_ptr<SymbolTable> now) override;
	void Set_IRV(int start_point) override;
	void Dump_IR(char* IR) const override;
};

// AddExp ::= MulExp | AddExp AddOp MulExp;
class AddExpAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> addexp;
	std::unique_ptr<BaseAST> addop;
	std::unique_ptr<BaseAST> mulexp;
	AddExpAST();
	virtual ~AddExpAST()override;
	int calculate() override;
	virtual void set_symbol_table(std::shared_ptr<SymbolTable> now) override;
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
	MulExpAST();
	virtual ~MulExpAST() override;
	virtual void set_symbol_table(std::shared_ptr<SymbolTable> now) override;
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
	RelExpAST();
	virtual  ~RelExpAST()override;
	int calculate() override;
	virtual void set_symbol_table(std::shared_ptr<SymbolTable> now) override;
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
	EqExpAST();
	virtual ~EqExpAST() override;
	int calculate() override;
	virtual void set_symbol_table(std::shared_ptr<SymbolTable> now) override;
	void Set_IRV(int start_point) override;
	void Dump_IR(char* IR) const override;
};

// LAndExp ::= EqExp | LAndExp "&&" EqExp;
class LAndExpAST : public BaseAST
{
public:
	std::unique_ptr<BaseAST> eqexp;
	std::unique_ptr<BaseAST> landexp;
	LAndExpAST();
	virtual ~LAndExpAST()override;
	int calculate() override;
	virtual void set_symbol_table(std::shared_ptr<SymbolTable> now) override;
	void Set_IRV(int start_point) override;
	void Dump_IR(char* IR) const override;
};

// LOrExp ::= LAndExp | LOrExp "||" LAndExp;
class LOrExpAST : public BaseAST
{
public:
	std::unique_ptr<BaseAST> landexp;
	std::unique_ptr<BaseAST> lorexp;
	LOrExpAST();
	virtual ~LOrExpAST() override;
	int calculate() override;
	virtual void set_symbol_table(std::shared_ptr<SymbolTable> now) override;
	void Set_IRV(int start_point) override;
	void Dump_IR(char* IR) const override;

};

// UnaryOp ::= "+" | "-" | "!";
class UnaryOpAST : public BaseAST {
public:
	UnaryOpAST();
	virtual ~UnaryOpAST() override;
	void Dump_IR(char* IR) const override;
};

// AddOp ::= "+" | "-"
class AddOpAST : public BaseAST {
public:
	AddOpAST();
	virtual ~AddOpAST() override;
	void Dump_IR(char* IR) const override;
};

// MulOp ::= "*" | "/" | "%"
class MulOpAST : public BaseAST {
public:
	MulOpAST();
	virtual ~MulOpAST();
	void Dump_IR(char* IR) const override;
};
// ("<" | ">" | "<=" | ">=")
class RelOpAST : public BaseAST {
public:
	RelOpAST();
	virtual ~RelOpAST();
	void Dump_IR(char* IR) const override;

};

//("==" | "!=")
class EqOpAST : public BaseAST {
public:
	EqOpAST();
	virtual ~EqOpAST();
	void Dump_IR(char* IR) const override;
};

