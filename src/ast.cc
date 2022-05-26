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

#define Integer 0
#define Register 1

SymbolTable symbol_table;

void CompUnitAST::Set_IRV(int start_point) {
	func_def->Set_IRV(start_point);
}

void CompUnitAST::Dump_IR(char* IR) const {
	func_def->Dump_IR(IR);
}

void CompUnitAST::set_symbol_table() {
	func_def->set_symbol_table();
}

void CompUnitAST::output_symbol_table() {
	cout << symbol_table << endl;
}
void FuncDefAST::Set_IRV(int start_point) {
	block->Set_IRV(start_point);
}

void FuncDefAST::set_symbol_table() {
	block->set_symbol_table();
}

void FuncDefAST::Dump_IR(char* IR) const {
	strcat(IR, "fun @");
	strcat(IR, const_cast<char*>(ident.c_str()));
	strcat(IR, "(): ");
	func_type->Dump_IR(IR);
	block->Dump_IR(IR);
}

void FuncTypeAST::Dump_IR(char* IR) const {
	if (type == "int") {
		strcat(IR, "i32 ");
	}
}

FuncTypeAST::FuncTypeAST(string _type):type(_type) {}
FuncTypeAST::FuncTypeAST() {}

// BlockAST::='{' '}'|'{' BlockItemVec '}'
void BlockAST::Set_IRV(int start_point) {
	if (flag == 1) {
		blockitemvec->Set_IRV(start_point);
	}
}

void BlockAST::set_symbol_table() {
	if (flag == 1) {
		blockitemvec->set_symbol_table();
	}
}
void BlockAST::Dump_IR(char* IR) const {
	strcat(IR, "{\n\%entry:\n");
	if (flag == 1) {
		blockitemvec->Dump_IR(IR);
	}
	strcat(IR, "}\n");
}

//BlockItemVec::=BlockItemVec BlockItem | BlockItem
void BlockItemVecAST::set_symbol_table() {
	for (int i = 0; i < itemvec.size(); i++) {
		itemvec[i]->set_symbol_table();
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
		start_point = itemvec[i]->IRV.return_value + 1;
	}
	IRV = itemvec[i - 1]->IRV;
}



// BlockItem::=Decl|Stmt
void BlockItemAST::set_symbol_table() {
	if (flag == 0) {
		decl->set_symbol_table();
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



// Decl::=ConstDecl
void DeclAST::set_symbol_table() {
	if (flag == 0) {
		constdecl->set_symbol_table();
	}
}

void DeclAST::Dump_IR(char* IR) const {
	if (flag == 0) {
		constdecl->Dump_IR(IR);
	}
}



// ConstDecl::="const" BType ConstDefVec ';'
void ConstDeclAST::set_symbol_table() {
	constdefvec->set_symbol_table();
}

void ConstDeclAST::Dump_IR(char* IR) const {
	return;//似乎不需要输出IR，只需要在符号表里记录
}


// ConstDefVec::= ConstDefVec ',' ConstDef | ConstDef

void ConstDefVecAST::set_symbol_table() {
	for (int i = 0; i < itemvec.size(); i++) {
		itemvec[i]->set_symbol_table();
	}
}


// ConstDefAST::= IDENT '=' ConstInitVal
int ConstDefAST::calculate() {
	return constinitval->calculate();
}
void ConstDefAST::set_symbol_table() {
	symbol_table.insert(ident, calculate());
}

// ConstInitVal::= ConstExp
int ConstInitValAST::calculate() {
	return constexp->calculate();
}

// LVal::= IDENT
int LValAST::lookup_table() {
	return symbol_table.find(ident);
}

// ConstExp ::= Exp
int ConstExpAST::calculate() {
	return exp->calculate();
}


// StmtAST::= RETURN Exp ';' | RETURN LVal ';' | Lval '=' Exp ';'
void StmtAST::Set_IRV(int start_point) {
	if (IRV.return_type != -1) return;
	if (flag == 0) {
		exp->Set_IRV(start_point);
		IRV = exp->IRV;
	}
}
void StmtAST::Dump_IR(char* IR) const {
	if (flag == 0) {
		exp->Dump_IR(IR);
		strcat(IR, "  ret ");
		if (this->IRV.return_type == Integer) {
			strcat(IR, const_cast<char*>(std::to_string(IRV.return_value).c_str()));
			strcat(IR, "\n");
		}
		else if (IRV.return_type == Register) {
			strcat(IR, const_cast<char*>(("%" + std::to_string(IRV.return_value) + "\n").c_str()));
		}
	}
}

// Exp ::= LOrExp
int ExpAST::calculate() {
	return lorexp->calculate();
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

// UnaryExp ::= PrimaryExp | UnaryOp UnaryExp
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

// PrimaryExp ::= "(" Exp ")" | Number| LVal not yet
int PrimaryExpAST::calculate() {
	if (flag == 0) return exp->calculate();
	if (flag == 1) return number;
	if (flag == 2) return lval->lookup_table();
}
void PrimaryExpAST::Set_IRV(int start_point) {
	if (IRV.return_type != -1) return;
	if (flag == 0) {
		exp->Set_IRV(start_point);
		IRV = exp->IRV; // 保持一致
	}
	else if (flag == 1)
	{
		IRV.return_type = Integer;
		IRV.return_value = number;
	}
}
void PrimaryExpAST::Dump_IR(char* IR) const {
	// Set_IRV(); // 结构已经完全推断，可以直接set
	if (flag == 0) {
		exp->Dump_IR(IR);
	}
}

// AddExp ::= MulExp | AddExp AddOp MulExp;
int AddExpAST::calculate() {
	if (flag == 0) return mulexp->calculate();
	else if (flag == 1) {
		if (addop->flag == 0) return addexp->calculate() + mulexp->calculate();
		else if (addop->flag == 1) return addexp->calculate() - mulexp->calculate();
	}
}
void AddExpAST::Set_IRV(int start_point) {
	if (IRV.return_type != -1) return;
	if (flag == 0) {
		mulexp->Set_IRV(start_point);
		IRV = mulexp->IRV;
	}
	else if (flag == 1) {
		mulexp->Set_IRV(start_point);
		if (mulexp->IRV.return_type == Register) {
			start_point = mulexp->IRV.return_value + 1; // 如果前半部分用到寄存器，则从下一个开始
		}
		// 计算顺序是先乘法再加法
		addexp->Set_IRV(start_point);
		if (addexp->IRV.return_type == Register) {
			start_point = addexp->IRV.return_value + 1; // 这里同理
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
		mulexp->Dump_IR(IR);
		addexp->Dump_IR(IR);
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

// MulExp ::=UnaryExp | MulExp MulOp UnaryExp
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
void MulExpAST::Set_IRV(int start_point) {
	if (IRV.return_type != -1)return;
	if (flag == 0) {
		unaryexp->Set_IRV(start_point);
		IRV = unaryexp->IRV;
	}
	else if (flag == 1) {
		// 先计算unaryexp 再计算乘法
		unaryexp->Set_IRV(start_point);
		if (unaryexp->IRV.return_type == Register) {
			start_point = unaryexp->IRV.return_value + 1;
		}
		mulexp->Set_IRV(start_point);
		if (mulexp->IRV.return_type == Register) {
			start_point = mulexp->IRV.return_value + 1;
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
		unaryexp->Dump_IR(IR); //先unaryexp计算
		mulexp->Dump_IR(IR);
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

// RelExp ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
int RelExpAST::calculate() {
	if (flag == 0) return addexp->flag;
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
void RelExpAST::Set_IRV(int start_point) {
	if (IRV.return_type != -1)return;
	if (flag == 0) {
		addexp->Set_IRV(start_point);
		IRV = addexp->IRV;
	}
	else if (flag == 1) {
		addexp->Set_IRV(start_point);
		if (addexp->IRV.return_type == Register) {
			start_point = addexp->IRV.return_value + 1;
		}
		relexp->Set_IRV(start_point);
		if (relexp->IRV.return_type == Register) {
			start_point = relexp->IRV.return_value + 1;
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
		addexp->Dump_IR(IR);
		relexp->Dump_IR(IR);
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
void EqExpAST::Set_IRV(int start_point) {
	if (IRV.return_type != -1)return;
	if (flag == 0) {
		relexp->Set_IRV(start_point);
		IRV = relexp->IRV;
	}
	else if (flag == 1) {
		relexp->Set_IRV(start_point);
		if (relexp->IRV.return_type == Register) {
			start_point = relexp->IRV.return_value + 1;
		}
		eqexp->Set_IRV(start_point);
		if (eqexp->IRV.return_type == Register) {
			start_point = eqexp->IRV.return_value + 1;
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
		relexp->Dump_IR(IR);
		eqexp->Dump_IR(IR);
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
int LAndExpAST::calculate() {
	if (flag == 0) {
		return eqexp->calculate();
	}
	else if (flag == 1) {
		return landexp->calculate() && eqexp->calculate();
	}
}
void LAndExpAST::Set_IRV(int start_point) {
	if (IRV.return_type != -1)return;
	if (flag == 0) {
		eqexp->Set_IRV(start_point);
		IRV = eqexp->IRV;
	}
	else if (flag == 1) {
		eqexp->Set_IRV(start_point);
		if (eqexp->IRV.return_type == Register) {
			start_point = eqexp->IRV.return_value + 1;
		}
		landexp->Set_IRV(start_point);
		if (landexp->IRV.return_type == Register) {
			start_point = landexp->IRV.return_value + 1;
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
		eqexp->Dump_IR(IR);
		landexp->Dump_IR(IR);
		std::string temp_IR = "  " + IRV.get_IR_value(-2) + " = ne " + landexp->IRV.get_IR_value() + ", 0\n";
		temp_IR += "  " + IRV.get_IR_value(-1) + " = ne " + eqexp->IRV.get_IR_value() + ", 0\n";
		temp_IR += "  " + IRV.get_IR_value(0) + " = and " + IRV.get_IR_value(-2) + ", " + IRV.get_IR_value(-1) + "\n";
		strcat(IR, const_cast<char*>(temp_IR.c_str()));
	}
}

// LOrExp ::= LAndExp | LOrExp "||" LAndExp;
int LOrExpAST::calculate() {
	if (flag == 0) return landexp->calculate();
	else if (flag == 1) {
		return lorexp->calculate() || landexp->calculate();
	}
}

void LOrExpAST::Set_IRV(int start_point) {
	if (IRV.return_type != -1)return;
	if (flag == 0) {
		landexp->Set_IRV(start_point);
		IRV = landexp->IRV;
	}

	else if (flag == 1) {
		landexp->Set_IRV(start_point);
		if (landexp->IRV.return_type == Register) {
			start_point = landexp->IRV.return_value + 1;
		}
		lorexp->Set_IRV(start_point);
		if (lorexp->IRV.return_type == Register) {
			start_point = lorexp->IRV.return_value + 1;
		}
		IRV.return_type = Register;
		IRV.return_value = start_point + 1;// 用位运算拼凑
	}
}

void LOrExpAST::Dump_IR(char* IR) const {
	if (flag == 0) {
		landexp->Dump_IR(IR);
	}
	else if (flag == 1) {
		landexp->Dump_IR(IR);
		lorexp->Dump_IR(IR);
		std::string temp_IR = "  " + IRV.get_IR_value(-1) + " = ";
		temp_IR += "or ";
		temp_IR += (lorexp->IRV.get_IR_value() + ", " + landexp->IRV.get_IR_value() + "\n");
		temp_IR += ("  " + IRV.get_IR_value(0) + " = ");
		temp_IR += ("ne " + IRV.get_IR_value(-1) + ", 0\n");
		strcat(IR, const_cast<char*>(temp_IR.c_str()));
	}
}

void UnaryOpAST::Dump_IR(char* IR) const { assert(false); }// UnaryOp ::= "+" | "-" | "!";
void AddOpAST::Dump_IR(char* IR) const { assert(false); }// AddOp ::= "+" | "-"
void MulOpAST::Dump_IR(char* IR) const { assert(false); }// MulOp ::= "*" | "/" | "%"
void RelOpAST::Dump_IR(char* IR) const { assert(false); }// ("<" | ">" | "<=" | ">=")
void EqOpAST::Dump_IR(char* IR) const { assert(false); }//("==" | "!=")

