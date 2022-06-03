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
	void set_value(int r_type, int r_value) {
		return_type = r_type;
		return_value = r_value;
	}
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
	//无论是常量还是变量都只有在声明后才设置为alive
	Varient(std::string _name, int _tag, int _value) :name(_name), tag(_tag), value(_value) {}
	Varient& operator=(const Varient& a) {
		name = a.name;
		tag = a.tag;
		value = a.value;
		return *this;
	}

	string get_str_value() {
		if (tag == ConstVar) {
			return std::to_string(value);
		}
		else if (tag == Var) {
			return "@" + name + "_" + std::to_string(value);
		}
		return "should not be here";
	}

}Varient;

// 每一个ast都包含一个对应层次的symboltable，里面仅存放当前层数的 not yet
class SymbolTable {
public:
	std::map<std::string, Varient> varient_pair_map;
	std::shared_ptr<SymbolTable> pre_table;

	static std::map<std::string, int> var_id;//保存所有变量的id，共享

	SymbolTable(std::shared_ptr<SymbolTable> _pre) :pre_table(_pre) {}

	// 按照tag来insert
	void insert(std::string key, int tag, int value) {
		if (tag == ConstVar) {
			varient_pair_map.insert(std::pair<std::string, Varient>(key, Varient(key, tag, value)));
		}
		else if (tag == Var) {
			if (var_id.find(key) == var_id.end()) {
				value = 0;
				var_id.insert(std::pair<std::string, int>(key, 0));
			}
			else {
				value = var_id.find(key)->second + 1;
				var_id[key] = value;
			}
			varient_pair_map.insert(std::pair<std::string, Varient>(key, Varient(key, tag, value)));
		}
	}
	//insert在dump的时候才加入，所以只要在table里面就证明处于alive的周期
	Varient* search_until_root(std::string key) {
		std::map<std::string, Varient> ::iterator l_it = varient_pair_map.find(key);
		if (l_it != varient_pair_map.end()) {
			return &(l_it->second);
		}
		else if (pre_table != nullptr) {
			return pre_table->search_until_root(key);
		}
		assert(false);//一定能搜到，语法正确的情况下
	}


	friend std::ostream& operator<<(std::ostream& out, SymbolTable& st) {
		for (auto iter = st.varient_pair_map.begin(); iter != st.varient_pair_map.end(); iter++) {
			out << "key " << iter->first << " " << iter->second << "\n";
		}
		return out;
	}
};


// 所有 AST 的基类，我们可以理解为一个节点，可以是叶节点或者内部节点，如果是叶节点表示终结符
class BaseAST {
public:
	int flag;
	IR_Ins_Value IRV;
	BaseAST() :flag(-1) {}
	virtual ~BaseAST() {};

	virtual void  Dump_IR(char* IR) {};
	virtual int calculate() { assert(false); return 0; };

};

class CompUnitAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> func_def;
	CompUnitAST();
	virtual ~CompUnitAST() override;

	virtual void Dump_IR(char* IR) override;
};

class FuncDefAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> func_type;
	std::string ident; // 这里没写构造函数，是后来赋值的
	std::unique_ptr<BaseAST> block;
	FuncDefAST();
	virtual ~FuncDefAST() override;

	virtual void Dump_IR(char* IR) override;
};

// BlockAST::='{' '}'|'{' BlockItemVec '}' //not yet
class BlockAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> blockitemvec;
	BlockAST();
	virtual ~BlockAST() override;

	virtual void Dump_IR(char* IR) override;
};

class FuncTypeAST :public BaseAST
{
public:
	std::string type;
	FuncTypeAST(std::string _type);
	FuncTypeAST();
	~FuncTypeAST() override;
	virtual void Dump_IR(char* IR) override;
};



//BlockItemVec::=BlockItemVec BlockItem | BlockItem
class BlockItemVecAST : public BaseAST {
public:
	std::vector<unique_ptr<BaseAST>> itemvec;
	BlockItemVecAST();
	virtual ~BlockItemVecAST() override;
	virtual void Dump_IR(char* IR) override;

};

// BlockItem::=Decl|Stmt
class BlockItemAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> decl;
	std::unique_ptr<BaseAST> stmt;
	BlockItemAST();
	virtual~BlockItemAST() override;
	virtual void Dump_IR(char* IR) override;


};

// Decl::=ConstDecl|VarDecl
class DeclAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> constdecl;
	std::unique_ptr<BaseAST> vardecl;
	DeclAST();
	virtual ~DeclAST()override;
	virtual void Dump_IR(char* IR) override;



};

// COnstDecl::="const" BType ConstDefVec ';' const的定义不需要irv设定或者dumpir
class ConstDeclAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> btype;
	std::unique_ptr<BaseAST> constdefvec;
	ConstDeclAST();
	virtual ~ConstDeclAST()override;

	virtual void Dump_IR(char* IR) override;
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

	virtual void Dump_IR(char* IR) override;

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
	virtual void Dump_IR(char* IR) override;

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

	virtual void Dump_IR(char* IR)override;
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

	virtual void Dump_IR(char* IR) override;
};

//VarDefVec::=VarDefVec ',' VarDef | VarDef
class VarDefVecAST : public BaseAST {
public:
	std::vector<std::unique_ptr<BaseAST>> itemvec;
	VarDefVecAST();
	virtual ~VarDefVecAST() override;

	virtual void Dump_IR(char* IR) override;
};

// VarDef::=IDENT|IDENT '=' InitVal
class VarDefAST : public BaseAST {
public:
	std::string ident;
	std::unique_ptr<BaseAST> initval;
	VarDefAST();
	virtual  ~VarDefAST() override;

	virtual void Dump_IR(char* IR) override;
};

// InitVal::=Exp
class InitValAST :public BaseAST {
public:
	std::unique_ptr<BaseAST> exp;
	InitValAST();
	virtual ~InitValAST() override;
	virtual void Dump_IR(char* IR) override;

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

	virtual void Dump_IR(char* IR) override;
};

class MatchedStmtAST : public BaseAST {
	std::unique_ptr<BaseAST> exp;
	std::unique_ptr<BaseAST>matched_stmt1;
	std::unique_ptr<BaseAST>matched_stmt2;
	std::unique_ptr<BaseAST>other_stmt;
	MatchedStmtAST();
	virtual  ~MatchedStmtAST();
	virtual void Dump_IR(char* IR) override;
};

class OpenStmtAST :public BaseAST {
	std::unique_ptr<BaseAST> exp;
	std::unique_ptr<BaseAST>matched_stmt;
	std::unique_ptr<BaseAST>open_stmt;
	std::unique_ptr<BaseAST>stmt;

	OpenStmtAST();
	virtual ~OpenStmtAST();
	virtual void Dump_IR(char* IR) override;
};

class OtherStmtAST : public BaseAST {
	std::unique_ptr<BaseAST> exp;
	std::unique_ptr<BaseAST> lval;
	std::unique_ptr<BaseAST> expexist;
	std::unique_ptr<BaseAST> block;
	OtherStmtAST();
	virtual  ~OtherStmtAST();
	virtual void Dump_IR(char* IR) override;
};


// ExpExist::Exp|ε
class ExpExistAST : public BaseAST {
public:
	std::unique_ptr<BaseAST>exp;
	ExpExistAST();
	virtual ~ExpExistAST()override;

	virtual void Dump_IR(char* IR) override;
};

// Exp ::= LOrExp
class ExpAST : public BaseAST {
public:
	std::unique_ptr<BaseAST> lorexp;
	ExpAST();
	virtual ~ExpAST();
	int calculate() override;
	int for_branch;//用来记录是否是为了分支

	void Dump_IR(char* IR) override;
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

	void Dump_IR(char* IR) override;
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

	void Dump_IR(char* IR) override;
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

	void Dump_IR(char* IR) override;
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

	void Dump_IR(char* IR) override;
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

	void Dump_IR(char* IR) override;
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


	void Dump_IR(char* IR) override;
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

	void Dump_IR(char* IR) override;
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

	void Dump_IR(char* IR) override;

};

// UnaryOp ::= "+" | "-" | "!";
class UnaryOpAST : public BaseAST {
public:
	UnaryOpAST();
	virtual ~UnaryOpAST() override;
	void Dump_IR(char* IR) override;
};

// AddOp ::= "+" | "-"
class AddOpAST : public BaseAST {
public:
	AddOpAST();
	virtual ~AddOpAST() override;
	void Dump_IR(char* IR) override;
};

// MulOp ::= "*" | "/" | "%"
class MulOpAST : public BaseAST {
public:
	MulOpAST();
	virtual ~MulOpAST();
	void Dump_IR(char* IR) override;
};
// ("<" | ">" | "<=" | ">=")
class RelOpAST : public BaseAST {
public:
	RelOpAST();
	virtual ~RelOpAST();
	void Dump_IR(char* IR) override;

};

//("==" | "!=")
class EqOpAST : public BaseAST {
public:
	EqOpAST();
	virtual ~EqOpAST();
	void Dump_IR(char* IR) override;
};

