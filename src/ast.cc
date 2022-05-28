#pragma once
#include<iostream>
#include <memory>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <assert.h>
#include "ast.h"
using namespace std;


CompUnitAST::CompUnitAST() {}
CompUnitAST::~CompUnitAST() {}

void CompUnitAST::Set_IRV(int start_point) {
	func_def->Set_IRV(start_point);
	IRV = func_def->IRV;
}

void CompUnitAST::Dump_IR(char* IR) const {
	func_def->Dump_IR(IR);
}

void CompUnitAST::set_symbol_table(std::shared_ptr<SymbolTable> now) {
	assert(now == nullptr);// 程序的开始 是一个nullptr
	symtable = std::shared_ptr<SymbolTable>(new SymbolTable());
	func_def->set_symbol_table(symtable);
}

void CompUnitAST::output_symbol_table() {
	cout << "output symbol table\n" << symtable << endl;
}

FuncDefAST::FuncDefAST() {}
FuncDefAST::~FuncDefAST() {}
void FuncDefAST::Set_IRV(int start_point) {
	block->Set_IRV(start_point);
	IRV = block->IRV;
}

void FuncDefAST::set_symbol_table(std::shared_ptr<SymbolTable> now) {
	symtable = now;
	block->set_symbol_table(symtable); // 此时不新建table，沿用上层的，stmt的时候新建table并设置指针
}

void FuncDefAST::Dump_IR(char* IR) const {
	strcat(IR, "fun @");
	strcat(IR, const_cast<char*>(ident.c_str()));
	strcat(IR, "(): ");
	func_type->Dump_IR(IR);
	strcat(IR, "\%entry:\n");
	block->Dump_IR(IR);
	strcat(IR, "}\n");
}

void FuncTypeAST::Dump_IR(char* IR) const {
	if (type == "int") {
		strcat(IR, "i32 {\n");
	}
}

FuncTypeAST::FuncTypeAST(string _type) :type(_type) {}
FuncTypeAST::FuncTypeAST() {}
FuncTypeAST::~FuncTypeAST() {}

// BlockAST::='{' '}'|'{' BlockItemVec '}'
BlockAST::BlockAST() {}
BlockAST::~BlockAST() {}
void BlockAST::Set_IRV(int start_point) {
	if (flag == 1) {
		blockitemvec->Set_IRV(start_point);
		IRV = blockitemvec->IRV;
	}
}

void BlockAST::set_symbol_table(std::shared_ptr<SymbolTable>now) {
	if (flag == 1) {
		symtable = now;
		blockitemvec->set_symbol_table(now);
	}
}
void BlockAST::Dump_IR(char* IR) const {

	if (flag == 1) {
		blockitemvec->Dump_IR(IR);
	}
}

//BlockItemVec::=BlockItemVec BlockItem | BlockItem
BlockItemVecAST::BlockItemVecAST() {}
BlockItemVecAST::~BlockItemVecAST() {}
void BlockItemVecAST::set_symbol_table(std::shared_ptr<SymbolTable>now) {
	symtable = now;
	for (int i = 0; i < itemvec.size(); i++) {
		itemvec[i]->set_symbol_table(symtable);
	}
}

void BlockItemVecAST::Dump_IR(char* IR) const {
	for (int i = 0; i < itemvec.size(); i++) {
		itemvec[i]->Dump_IR(IR);
	}
}

void BlockItemVecAST::Set_IRV(int start_point) {
	int i = 0;
	for (i = 0; i < itemvec.size(); i++) {
		itemvec[i]->Set_IRV(start_point);
		if (itemvec[i]->IRV.return_type == Register) {
			start_point = itemvec[i]->IRV.return_value + 1;
			IRV = itemvec[i]->IRV;
		}
	}
}



// BlockItem::=Decl|Stmt
BlockItemAST::BlockItemAST() {}
BlockItemAST::~BlockItemAST() {}
void BlockItemAST::set_symbol_table(std::shared_ptr<SymbolTable>now) {
	symtable = now;
	if (flag == 0) {
		decl->set_symbol_table(symtable);
	}
	else if (flag == 1) {
		stmt->set_symbol_table(symtable);
	}
}

void BlockItemAST::Dump_IR(char* IR) const {
	if (flag == 0) {
		decl->Dump_IR(IR);
	}
	else if (flag == 1) {
		stmt->Dump_IR(IR);
	}
}
void BlockItemAST::Set_IRV(int start_point) {
	if (flag == 0) {
		decl->Set_IRV(start_point);
		IRV = decl->IRV;
	}
	else if (flag == 1) {
		stmt->Set_IRV(start_point);
		IRV = stmt->IRV;
	}
}



// Decl::=ConstDecl|VarDecl
DeclAST::DeclAST() {}
DeclAST::~DeclAST() {}
void DeclAST::set_symbol_table(std::shared_ptr<SymbolTable>now) {
	symtable = now;
	if (flag == 0) {
		constdecl->set_symbol_table(symtable);
	}
	else if (flag == 1) {
		vardecl->set_symbol_table(symtable);
	}
}

void DeclAST::Dump_IR(char* IR) const {
	if (flag == 0) {}// const定义不用dump
	else if (flag == 1) {
		vardecl->Dump_IR(IR);
	}
}

void DeclAST::Set_IRV(int start_point) {
	if (flag == 0) {}// const定义 不用设定IRV
	else if (flag == 1) {
		vardecl->Set_IRV(start_point);
		IRV = vardecl->IRV;
	}
}


// ConstDecl::="const" BType ConstDefVec ';'
ConstDeclAST::ConstDeclAST() {}
ConstDeclAST::~ConstDeclAST() {}
void ConstDeclAST::set_symbol_table(std::shared_ptr<SymbolTable>now) {
	symtable = now;
	constdefvec->set_symbol_table(symtable);
}

// ConstDefVec::= ConstDefVec ',' ConstDef | ConstDef
ConstDefVecAST::ConstDefVecAST() {}
ConstDefVecAST::~ConstDefVecAST() {}
void ConstDefVecAST::set_symbol_table(std::shared_ptr<SymbolTable>now) {
	symtable = now;
	for (int i = 0; i < itemvec.size(); i++) {
		itemvec[i]->set_symbol_table(symtable);
	}
}
BtypeAST::BtypeAST() {}
BtypeAST::~BtypeAST() {}

// ConstDefAST::= IDENT '=' ConstInitVal
ConstDefAST::ConstDefAST() {}
ConstDefAST::~ConstDefAST() {}
int ConstDefAST::calculate() {
	return constinitval->calculate();
}
void ConstDefAST::set_symbol_table(std::shared_ptr<SymbolTable>now) {
	symtable = now;
	symtable->insert(ident, ConstVar, calculate());
}

// ConstInitVal::= ConstExp
ConstInitValAST::ConstInitValAST() {}
ConstInitValAST::~ConstInitValAST() {}
int ConstInitValAST::calculate() {
	return constexp->calculate();
}

// LVal::= IDENT
LValAST::LValAST() {}
LValAST::~LValAST() {}
void LValAST::set_symbol_table(std::shared_ptr<SymbolTable>now) {
	symtable = now;
}

void::LValAST::Set_IRV(int start_point) {
	Varient v = symtable->search_until_root(ident);
	if (v.tag == ConstVar) {
		IRV.return_type = Integer;
		IRV.return_value = v.value;
	}
	else if (v.tag == Var) { //load 到寄存器
		IRV.return_type = Register;
		IRV.return_value = start_point;
	}
}

// ConstExp ::= Exp 实现了计算函数，能返回只给def就可
ConstExpAST::ConstExpAST() {}
ConstExpAST::~ConstExpAST() {}
int ConstExpAST::calculate() {
	return exp->calculate();
}


// VarDecl::=Btype VarDefVec ';'
VarDeclAST::VarDeclAST() {}
VarDeclAST::~VarDeclAST() {}
void VarDeclAST::set_symbol_table(std::shared_ptr<SymbolTable>now) {
	symtable = now;
	vardefvec->set_symbol_table(symtable);
}

void VarDeclAST::Set_IRV(int start_point) {
	vardefvec->Set_IRV(start_point);
	IRV = vardefvec->IRV;
}

void VarDeclAST::Dump_IR(char* IR) const {
	vardefvec->Dump_IR(IR);
}

// VarDefVec::=VarDefVec ',' VarDef | VarDef
VarDefVecAST::VarDefVecAST() {}
VarDefVecAST::~VarDefVecAST() {}

void VarDefVecAST::set_symbol_table(std::shared_ptr<SymbolTable>now) {
	symtable = now;
	for (int i = 0; i < itemvec.size(); ++i) {
		itemvec[i]->set_symbol_table(symtable);
	}
}

void VarDefVecAST::Set_IRV(int start_point) {
	int i = 0;
	for (i = 0; i < itemvec.size(); ++i) {
		itemvec[i]->Set_IRV(start_point);
		if (itemvec[i]->IRV.return_type == Register) {
			start_point = itemvec[i]->IRV.return_value + 1;
			IRV = itemvec[i]->IRV;
		}
	}
}
void VarDefVecAST::Dump_IR(char* IR)const {
	for (int i = 0; i < itemvec.size(); ++i) {
		itemvec[i]->Dump_IR(IR);
	}
}

// VarDef::=IDENT|IDENT '=' InitVal 插入一个新的变量到当层符号表中，要查询目前整棵树的符号表情况，决定变量的标号
VarDefAST::VarDefAST() {}
VarDefAST::~VarDefAST() {}
void VarDefAST::set_symbol_table(std::shared_ptr<SymbolTable>now) {
	symtable = now;
	cout << ident << " xxxxx" << endl;
	symtable->insert(ident, Var, 0);
	cout << symtable;
	if (flag == 1) {
		initval->set_symbol_table(symtable);
	}
}
void VarDefAST::Set_IRV(int start_point) {
	if (flag == 0) {}// 不操作
	if (flag == 1) {
		initval->Set_IRV(start_point);
		IRV = initval->IRV;
	}
}

void VarDefAST::Dump_IR(char* IR) const {
	std::string temp_IR = "  @" + ident + " = alloc i32\n";
	strcat(IR, const_cast<char*>(temp_IR.c_str()));
	if (flag == 1) {
		initval->Dump_IR(IR);
		temp_IR = "  store " + initval->IRV.get_IR_value() + ", " + symtable->search_until_root(ident).get_str_value(symtable->multi_def(ident)) + "\n";
		strcat(IR, const_cast<char*>(temp_IR.c_str()));
	}
}

// InitVal::=Exp
InitValAST::InitValAST() {}
InitValAST::~InitValAST() {}
void InitValAST::Set_IRV(int start_point) {
	exp->Set_IRV(start_point);
	IRV = exp->IRV;
}
void InitValAST::Dump_IR(char* IR) const {
	exp->Dump_IR(IR);
}

void InitValAST::set_symbol_table(std::shared_ptr<SymbolTable>now) {
	symtable = now;
	exp->set_symbol_table(symtable);
}


// StmtAST::=  Lval '=' Exp ';' | ExpExist ';' | Block | "return" ExpExist ';'
StmtAST::StmtAST() {}
StmtAST::~StmtAST() {}
void StmtAST::Set_IRV(int start_point) {
	if (IRV.return_type != -1) return;
	switch (flag)
	{
	case 0:
		exp->Set_IRV(start_point);
		IRV = exp->IRV; break;
	case 1:
		expexist->Set_IRV(start_point);
		IRV = expexist->IRV; break;
	case 2:
		block->Set_IRV(start_point);
		IRV = block->IRV; break;
	case 3:
		expexist->Set_IRV(start_point);
		IRV = expexist->IRV; break;
	}
}

void StmtAST::set_symbol_table(std::shared_ptr<SymbolTable> now) {
	switch (flag)
	{
	case 0:symtable = now; exp->set_symbol_table(symtable); break;
	case 1:symtable = now; expexist->set_symbol_table(symtable); break;
	case 2:
		symtable = now;
		now = std::shared_ptr<SymbolTable>(new SymbolTable());
		symtable->sons.push_back(now);
		now->pre = symtable;
		block->set_symbol_table(now);
		break;
	case 3:
		symtable = now; expexist->set_symbol_table(symtable); break;
	}
}

void StmtAST::Dump_IR(char* IR) const {
	switch (flag)
	{
	case 0: {
		exp->Dump_IR(IR);
		std::string key = dynamic_cast<LValAST&>(*lval).ident;
		std::string temp_IR = "  store " + IRV.get_IR_value() + ", " + symtable->search_until_root(key).get_str_value(symtable->multi_def(key)) + "\n";
		strcat(IR, const_cast<char*>(temp_IR.c_str()));
		break;
	}
	case 1: {
		expexist->Dump_IR(IR);
		break;
	}
	case 2: {
		block->Dump_IR(IR);
		break;
	}
	case 3: {
		if (expexist->flag == 0) {
			expexist->Dump_IR(IR);
			std::string temp_IR = "  ret " + IRV.get_IR_value() + "\n";
			strcat(IR, const_cast<char*>(temp_IR.c_str()));
		}
		else if (expexist->flag == 1) {
			strcat(IR, "  ret \n");
		}
		break;
	}
	}
}


//ExpExist ::= Exp|ε
ExpExistAST::ExpExistAST() {}
ExpExistAST::~ExpExistAST() {}
void ExpExistAST::Set_IRV(int start_point) {
	if (flag == 0) {
		exp->Set_IRV(start_point);
		IRV = exp->IRV;
	}
}

void ExpExistAST::set_symbol_table(std::shared_ptr<SymbolTable>now) {
	symtable = now;
	if (flag == 0) {
		exp->set_symbol_table(symtable);
	}
}

void ExpExistAST::Dump_IR(char* IR)const {
	if (flag == 0) {
		exp->Dump_IR(IR);
	}
}

// Exp ::= LOrExp
ExpAST::ExpAST() {}
ExpAST::~ExpAST() {}
int ExpAST::calculate() {
	return lorexp->calculate();
}

void ExpAST::set_symbol_table(std::shared_ptr<SymbolTable>now) {
	symtable = now;
	lorexp->set_symbol_table(now);
}

void ExpAST::Set_IRV(int start_point) {
	if (IRV.return_type != -1) return;
	if (flag == 0) {
		lorexp->Set_IRV(start_point);
		IRV = lorexp->IRV;
	}
}
void ExpAST::Dump_IR(char* IR) const {
	if (flag == 0) {
		lorexp->Dump_IR(IR);
	}
}



// PrimaryExp ::= "(" Exp ")" | Number| LVal
PrimaryExpAST::PrimaryExpAST() {}
PrimaryExpAST::~PrimaryExpAST() {}
int PrimaryExpAST::calculate() {
	if (flag == 0) return exp->calculate();
	if (flag == 1) return number;
	if (flag == 2) return lval->symtable->search_until_root(dynamic_cast<LValAST&>(*lval).ident).value;
}

void PrimaryExpAST::set_symbol_table(std::shared_ptr<SymbolTable>now) {
	symtable = now;
	switch (flag) {
	case 0:exp->set_symbol_table(now); break;
	case 1:break;
	case 2:lval->set_symbol_table(now); break;
	}
}

void PrimaryExpAST::Set_IRV(int start_point) {
	if (IRV.return_type != -1) return;
	switch (flag) {
	case 0: {
		exp->Set_IRV(start_point);
		IRV = exp->IRV;
		break;
	}
	case 1: {
		IRV.return_type = Integer;
		IRV.return_value = number;
		break;
	}
	case 2: { // 可能是register（由变量load而来）或者integer
		lval->Set_IRV(start_point);
		IRV = lval->IRV;
		break;
	}
	}
}
void PrimaryExpAST::Dump_IR(char* IR) const {
	if (flag == 0) {
		exp->Dump_IR(IR);
	}
	else if (flag == 2 && IRV.return_type == Register) { // load一个变量
		std::string key = dynamic_cast<LValAST&>(*lval).ident;
		std::string temp_IR = "  " + IRV.get_IR_value() + " = load " + symtable->search_until_root(key).get_str_value(symtable->multi_def(key)) + "\n";
		strcat(IR, const_cast<char*>(temp_IR.c_str()));
	}
}

// UnaryExp ::= PrimaryExp | UnaryOp UnaryExp
UnaryExpAST::UnaryExpAST() {}
UnaryExpAST::~UnaryExpAST() {}
int UnaryExpAST::calculate() {
	if (flag == 0) {
		return primaryexp->calculate();
	}
	else if (flag == 1) {
		switch (unaryop->flag) {
		case 0: return  unaryexp->calculate(); break;
		case 1: return -unaryexp->calculate(); break;
		case 2: return !unaryexp->calculate(); break;
		}
	}
}

void UnaryExpAST::set_symbol_table(std::shared_ptr<SymbolTable>now) {
	symtable = now;
	if (flag == 0) {
		primaryexp->set_symbol_table(now);
	}
	else if (flag == 1) {
		unaryexp->set_symbol_table(now);
	}

}

void UnaryExpAST::Set_IRV(int start_point) {
	if (IRV.return_type != -1) return;
	if (flag == 0) {
		primaryexp->Set_IRV(start_point); // 下层一定要先递归set之后才轮到下层
		IRV = primaryexp->IRV;
	}
	else if (flag == 1) { // 要分情况讨论分解为 unaryop unaryexp的时候，op和exp分别的状态
		unaryexp->Set_IRV(start_point);

		if (unaryop->flag == 0) { // "+"保持不变，不做这一步操作
			IRV = unaryexp->IRV;
		}
		else if (unaryop->flag == 1 || unaryop->flag == 2) { // 分类讨论上层的type
			if (unaryexp->IRV.return_type == Integer) {
				IRV.return_type = Register;
				IRV.return_value = start_point; //作为当前的第一个，从start point开始
			}
			else if (unaryexp->IRV.return_type == Register) {
				IRV.return_type = Register;
				IRV.return_value = unaryexp->IRV.return_value + 1;// 从下一个寄存器开始, 因为不是从当前起始点开始
			}
		}
	}

}
void UnaryExpAST::Dump_IR(char* IR) const {
	// Set_IRV(); // 总是要先set irv
	if (flag == 0) {
		primaryexp->Dump_IR(IR);
	}
	else if (flag == 1) {
		unaryexp->Dump_IR(IR);
		// "+" 不处理
		if (unaryop->flag == 0) {}
		else {
			std::string temp_IR = "  ";
			if (IRV.return_type == Register) temp_IR += ("%" + std::to_string(IRV.return_value));
			temp_IR += " = ";
			// "-" 分类处理
			if (unaryop->flag == 1) {
				temp_IR += "sub 0, ";
				if (unaryexp->IRV.return_type == Integer) {
					temp_IR += (std::to_string(unaryexp->IRV.return_value) + "\n");
				}
				else if (unaryexp->IRV.return_type == Register) {
					temp_IR += ("%" + std::to_string(unaryexp->IRV.return_value) + "\n");
				}
			}
			// "!" 单独处理
			else if (unaryop->flag == 2) {
				temp_IR += "eq ";
				if (unaryexp->IRV.return_type == Integer) {
					temp_IR += std::to_string(unaryexp->IRV.return_value);
				}
				else if (unaryexp->IRV.return_type == Register) {
					temp_IR += ("%" + std::to_string(unaryexp->IRV.return_value));
				}
				temp_IR += ", 0\n";

			}
			strcat(IR, const_cast<char*>(temp_IR.c_str()));
		}
	}

}


// MulExp ::=UnaryExp | MulExp MulOp UnaryExp
MulExpAST::MulExpAST() {}
MulExpAST::~MulExpAST() {}
int MulExpAST::calculate() {
	if (flag == 0) return unaryexp->calculate();
	else if (flag == 1) {
		switch (mulop->flag)
		{
		case 0: return mulexp->calculate() * unaryexp->calculate(); break;
		case 1: return mulexp->calculate() / unaryexp->calculate(); break;
		case 2: return mulexp->calculate() % unaryexp->calculate(); break;
		}
	}
}
void MulExpAST::set_symbol_table(std::shared_ptr<SymbolTable>now) {
	symtable = now;
	unaryexp->set_symbol_table(now);
	if (flag == 1)mulexp->set_symbol_table(now);
}
void MulExpAST::Set_IRV(int start_point) {
	if (IRV.return_type != -1)return;
	if (flag == 0) {
		unaryexp->Set_IRV(start_point);
		IRV = unaryexp->IRV;
	}
	else if (flag == 1) {
		mulexp->Set_IRV(start_point);
		if (mulexp->IRV.return_type == Register) {
			start_point = mulexp->IRV.return_value + 1;
		}
		unaryexp->Set_IRV(start_point);
		if (unaryexp->IRV.return_type == Register) {
			start_point = unaryexp->IRV.return_value + 1;
		}
		IRV.return_type = Register;
		IRV.return_value = start_point;
	}
}
void MulExpAST::Dump_IR(char* IR) const {
	if (flag == 0) {
		unaryexp->Dump_IR(IR);
	}
	else if (flag == 1) {
		mulexp->Dump_IR(IR);
		unaryexp->Dump_IR(IR);
		std::string temp_IR = "  " + IRV.get_IR_value() + " = ";
		switch (mulop->flag) {
		case 0: temp_IR += "mul "; break;
		case 1: temp_IR += "div "; break;
		case 2: temp_IR += "mod "; break;
		}
		temp_IR += (mulexp->IRV.get_IR_value() + ", " + unaryexp->IRV.get_IR_value() + "\n");
		strcat(IR, const_cast<char*>(temp_IR.c_str()));
	}
}

// AddExp ::= MulExp | AddExp AddOp MulExp;
AddExpAST::AddExpAST() {}
AddExpAST::~AddExpAST() {}
int AddExpAST::calculate() {
	if (flag == 0) return mulexp->calculate();
	else if (flag == 1) {
		if (addop->flag == 0) return addexp->calculate() + mulexp->calculate();
		else if (addop->flag == 1) return addexp->calculate() - mulexp->calculate();
	}
}

void AddExpAST::set_symbol_table(std::shared_ptr<SymbolTable>now) {
	symtable = now;
	mulexp->set_symbol_table(now);
	if (flag == 1)addexp->set_symbol_table(now);
}
void AddExpAST::Set_IRV(int start_point) {
	if (IRV.return_type != -1) return;
	if (flag == 0) {
		mulexp->Set_IRV(start_point);
		IRV = mulexp->IRV;
	}
	else if (flag == 1) {
		addexp->Set_IRV(start_point);
		if (addexp->IRV.return_type == Register) {
			start_point = addexp->IRV.return_value + 1;
		}
		mulexp->Set_IRV(start_point);
		if (mulexp->IRV.return_type == Register) {
			start_point = mulexp->IRV.return_value + 1;
		}
		IRV.return_type = Register;
		IRV.return_value = start_point;
	}
}
void AddExpAST::Dump_IR(char* IR) const {
	if (flag == 0) {
		mulexp->Dump_IR(IR);
	}
	else if (flag == 1) {
		addexp->Dump_IR(IR);
		mulexp->Dump_IR(IR);
		std::string temp_IR = "  " + IRV.get_IR_value() + " = ";
		if (addop->flag == 0) {
			temp_IR += ("add ");
		}
		else if (addop->flag == 1) {
			temp_IR += ("sub ");
		}
		temp_IR += (addexp->IRV.get_IR_value() + ", " + mulexp->IRV.get_IR_value() + "\n");
		strcat(IR, const_cast<char*>(temp_IR.c_str()));
	}
}


// RelExp ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
RelExpAST::RelExpAST() {}
RelExpAST::~RelExpAST() {}
int RelExpAST::calculate() {
	if (flag == 0) return addexp->calculate();
	if (flag == 1) {
		switch (relop->flag)
		{
		case 0: return relexp->calculate() < addexp->calculate(); break;
		case 1: return relexp->calculate() > addexp->calculate(); break;
		case 2: return relexp->calculate() <= addexp->calculate(); break;
		case 3: return relexp->calculate() >= addexp->calculate(); break;
		}
	}
}
void RelExpAST::set_symbol_table(std::shared_ptr<SymbolTable>now) {
	symtable = now;
	addexp->set_symbol_table(now);
	if (flag == 1)relexp->set_symbol_table(now);
}
void RelExpAST::Set_IRV(int start_point) {
	if (IRV.return_type != -1)return;
	if (flag == 0) {
		addexp->Set_IRV(start_point);
		IRV = addexp->IRV;
	}
	else if (flag == 1) {
		relexp->Set_IRV(start_point);
		if (relexp->IRV.return_type == Register) {
			start_point = relexp->IRV.return_value + 1;
		}
		addexp->Set_IRV(start_point);
		if (addexp->IRV.return_type == Register) {
			start_point = addexp->IRV.return_value + 1;
		}
		IRV.return_type = Register;
		IRV.return_value = start_point;
	}
}
void RelExpAST::Dump_IR(char* IR) const {
	if (flag == 0) {
		addexp->Dump_IR(IR);
	}
	else if (flag == 1) {
		relexp->Dump_IR(IR);
		addexp->Dump_IR(IR);
		std::string temp_IR = "  " + IRV.get_IR_value() + " = ";
		switch (relop->flag)
		{
		case 0: temp_IR += "lt "; break;
		case 1: temp_IR += "gt "; break;
		case 2: temp_IR += "le "; break;
		case 3: temp_IR += "ge "; break;
		}
		temp_IR += (relexp->IRV.get_IR_value() + ", " + addexp->IRV.get_IR_value() + "\n");
		strcat(IR, const_cast<char*>(temp_IR.c_str()));
	}
}

// EqExp ::= RelExp | EqExp ("==" | "!=") RelExp;
EqExpAST::EqExpAST() {}
EqExpAST::~EqExpAST() {}
int EqExpAST::calculate() {
	if (flag == 0) {
		return relexp->calculate();
	}
	else if (flag == 1) {
		if (eqop->flag == 0) {
			return eqexp->calculate() == relexp->calculate();
		}
		else if (eqop->flag == 1) {
			return eqexp->calculate() != relexp->calculate();
		}
	}
}
void EqExpAST::set_symbol_table(std::shared_ptr<SymbolTable>now) {
	symtable = now;
	relexp->set_symbol_table(now);
	if (flag == 1)eqexp->set_symbol_table(now);
}
void EqExpAST::Set_IRV(int start_point) {
	if (IRV.return_type != -1)return;
	if (flag == 0) {
		relexp->Set_IRV(start_point);
		IRV = relexp->IRV;
	}
	else if (flag == 1) {
		eqexp->Set_IRV(start_point);
		if (eqexp->IRV.return_type == Register) {
			start_point = eqexp->IRV.return_value + 1;
		}
		relexp->Set_IRV(start_point);
		if (relexp->IRV.return_type == Register) {
			start_point = relexp->IRV.return_value + 1;
		}
		IRV.return_type = Register;
		IRV.return_value = start_point;
	}
}
void EqExpAST::Dump_IR(char* IR) const {
	if (flag == 0) {
		relexp->Dump_IR(IR);
	}
	else if (flag == 1) {
		eqexp->Dump_IR(IR);
		relexp->Dump_IR(IR);
		std::string temp_IR = "  " + IRV.get_IR_value() + " = ";
		switch (eqop->flag)
		{
		case 0: temp_IR += "eq "; break;
		case 1: temp_IR += "ne "; break;
		}
		temp_IR += (eqexp->IRV.get_IR_value() + ", " + relexp->IRV.get_IR_value() + "\n");
		strcat(IR, const_cast<char*>(temp_IR.c_str()));
	}
}

// LAndExp ::= EqExp | LAndExp "&&" EqExp;
LAndExpAST::LAndExpAST() {}
LAndExpAST::~LAndExpAST() {}
int LAndExpAST::calculate() {
	if (flag == 0) {
		return eqexp->calculate();
	}
	else if (flag == 1) {
		return landexp->calculate() && eqexp->calculate();
	}
}

void LAndExpAST::set_symbol_table(std::shared_ptr<SymbolTable>now) {
	symtable = now;
	eqexp->set_symbol_table(now);
	if (flag == 1)landexp->set_symbol_table(now);
}
void LAndExpAST::Set_IRV(int start_point) {
	if (IRV.return_type != -1)return;
	if (flag == 0) {
		eqexp->Set_IRV(start_point);
		IRV = eqexp->IRV;
	}
	else if (flag == 1) {
		landexp->Set_IRV(start_point);
		if (landexp->IRV.return_type == Register) {
			start_point = landexp->IRV.return_value + 1;
		}
		eqexp->Set_IRV(start_point);
		if (eqexp->IRV.return_type == Register) {
			start_point = eqexp->IRV.return_value + 1;
		}
		IRV.return_type = Register;
		IRV.return_value = start_point + 2; //逻辑land拆解为三部，先两边分别和0取neq再and
	}
}
void LAndExpAST::Dump_IR(char* IR) const {
	if (flag == 0) {
		eqexp->Dump_IR(IR);
	}
	else if (flag == 1) {
		landexp->Dump_IR(IR);
		eqexp->Dump_IR(IR);
		std::string temp_IR = "  " + IRV.get_IR_value(-2) + " = ne " + landexp->IRV.get_IR_value() + ", 0\n";
		temp_IR += "  " + IRV.get_IR_value(-1) + " = ne " + eqexp->IRV.get_IR_value() + ", 0\n";
		temp_IR += "  " + IRV.get_IR_value(0) + " = and " + IRV.get_IR_value(-2) + ", " + IRV.get_IR_value(-1) + "\n";
		strcat(IR, const_cast<char*>(temp_IR.c_str()));
	}
}

// LOrExp ::= LAndExp | LOrExp "||" LAndExp;
LOrExpAST::LOrExpAST() {}
LOrExpAST::~LOrExpAST() {}
int LOrExpAST::calculate() {
	if (flag == 0) return landexp->calculate();
	else if (flag == 1) {
		return lorexp->calculate() || landexp->calculate();
	}
}

void LOrExpAST::set_symbol_table(std::shared_ptr<SymbolTable>now) {
	symtable = now;
	landexp->set_symbol_table(now);
	if (flag == 1)lorexp->set_symbol_table(now);
}

void LOrExpAST::Set_IRV(int start_point) {
	if (IRV.return_type != -1)return;
	if (flag == 0) {
		landexp->Set_IRV(start_point);
		IRV = landexp->IRV;
	}

	else if (flag == 1) {
		lorexp->Set_IRV(start_point);
		if (lorexp->IRV.return_type == Register) {
			start_point = lorexp->IRV.return_value + 1;
		}
		landexp->Set_IRV(start_point);
		if (landexp->IRV.return_type == Register) {
			start_point = landexp->IRV.return_value + 1;
		}

		IRV.return_type = Register;
		IRV.return_value = start_point + 1;// 用位运算拼凑 多使用一次寄存器
	}
}

void LOrExpAST::Dump_IR(char* IR) const {
	if (flag == 0) {
		landexp->Dump_IR(IR);
	}
	else if (flag == 1) {
		lorexp->Dump_IR(IR);
		landexp->Dump_IR(IR);
		std::string temp_IR = "  " + IRV.get_IR_value(-1) + " = ";
		temp_IR += "or ";
		temp_IR += (lorexp->IRV.get_IR_value() + ", " + landexp->IRV.get_IR_value() + "\n");
		temp_IR += ("  " + IRV.get_IR_value(0) + " = ");
		temp_IR += ("ne " + IRV.get_IR_value(-1) + ", 0\n");
		strcat(IR, const_cast<char*>(temp_IR.c_str()));
	}
}
UnaryOpAST::UnaryOpAST() {}
UnaryOpAST::~UnaryOpAST() {}
AddOpAST::AddOpAST() {}
AddOpAST::~AddOpAST() {}
MulOpAST::MulOpAST() {}
MulOpAST::~MulOpAST() {}
RelOpAST::RelOpAST() {}
RelOpAST::~RelOpAST() {}
EqOpAST::EqOpAST() {}
EqOpAST::~EqOpAST() {}
void UnaryOpAST::Dump_IR(char* IR) const { assert(false); }// UnaryOp ::= "+" | "-" | "!";
void AddOpAST::Dump_IR(char* IR) const { assert(false); }// AddOp ::= "+" | "-"
void MulOpAST::Dump_IR(char* IR) const { assert(false); }// MulOp ::= "*" | "/" | "%"
void RelOpAST::Dump_IR(char* IR) const { assert(false); }// ("<" | ">" | "<=" | ">=")
void EqOpAST::Dump_IR(char* IR) const { assert(false); }//("==" | "!=")

