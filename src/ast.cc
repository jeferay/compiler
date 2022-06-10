#include<iostream>
#include <memory>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <assert.h>
#include "ast.h"
using namespace std;

int Basic_Block::block_num = 0;
Basic_Block* now_block = NULL;
std::vector<Basic_Block*> bb_stack;
std::map<std::string, int> SymbolTable::var_id;//保存所有变量的id，共享
std::shared_ptr<SymbolTable> root_table = std::shared_ptr<SymbolTable>(new SymbolTable(NULL));
auto now_table = root_table;
int start_point;

Basic_Block* get_new_bb() {
	assert(now_block == NULL);
	if (bb_stack.size()) {
		Basic_Block* last = bb_stack[bb_stack.size() - 1];
		bb_stack.pop_back();
		return last;
	}
	else {
		return new Basic_Block(NULL, NULL);
	}
}



CompUnitAST::CompUnitAST() {}
CompUnitAST::~CompUnitAST() {}

void CompUnitAST::Dump_IR(char* IR, int last_sentence = 0) {

	func_def->Dump_IR(IR, last_sentence);
	IRV = func_def->IRV;
}


FuncDefAST::FuncDefAST() {}
FuncDefAST::~FuncDefAST() {}

void FuncDefAST::Dump_IR(char* IR, int last_sentence = 0) {
	strcat(IR, "fun @");
	strcat(IR, const_cast<char*>(ident.c_str()));
	strcat(IR, "(): ");
	func_type->Dump_IR(IR, last_sentence);
	block->Dump_IR(IR, last_sentence);
	if (now_block) {
		strcat(IR, "  ret 0\n\n");
	}
	IRV = block->IRV;
	strcat(IR, "}\n");
}

void FuncTypeAST::Dump_IR(char* IR, int last_sentence = 0) {
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

void BlockAST::Dump_IR(char* IR, int last_sentence = 0) {
	if (now_block == NULL) {
		now_block = get_new_bb();//并不入栈
		now_block->output_into_block(IR);
	}
	if (flag == 1) {
		blockitemvec->Dump_IR(IR, last_sentence);
		IRV = blockitemvec->IRV;
	}
}

//BlockItemVec::=BlockItemVec BlockItem | BlockItem
BlockItemVecAST::BlockItemVecAST() {}
BlockItemVecAST::~BlockItemVecAST() {}

void BlockItemVecAST::Dump_IR(char* IR, int last_sentence = 0) {
	for (int i = 0; i < itemvec.size(); i++) {
		int _last = (itemvec.size() - 1) == i;
		itemvec[i]->Dump_IR(IR, _last);
		if (now_block && now_block->dead) {
			delete now_block;
			now_block = NULL;
			break;
		}
		// 	std::string temp_IR = "  jump " + now_block->left->get_block_name() + "\n\n";
		// 	strcat(IR, const_cast<char*>(temp_IR.c_str()));
		// 	delete now_block;
		// 	now_block = NULL;
		// }
	}
}



// BlockItem::=Decl|Stmt
BlockItemAST::BlockItemAST() {}
BlockItemAST::~BlockItemAST() {}

//每一条语句重新判定一下，两个return
void BlockItemAST::Dump_IR(char* IR, int last_sentence = 0) {
	if (now_block == NULL) {
		now_block = get_new_bb();
		now_block->output_into_block(IR);
	}
	if (flag == 0) {
		decl->Dump_IR(IR, last_sentence);
	}
	else if (flag == 1) {
		stmt->Dump_IR(IR, last_sentence);
	}
}

// Decl::=ConstDecl|VarDecl
DeclAST::DeclAST() {}
DeclAST::~DeclAST() {}


void DeclAST::Dump_IR(char* IR, int last_sentence = 0) {
	if (flag == 0) {
		constdecl->Dump_IR(IR, last_sentence);
	}// const定义也需要定义dump IR 插入表格
	else if (flag == 1) {
		vardecl->Dump_IR(IR, last_sentence);
	}
}


// ConstDecl::="const" BType ConstDefVec ';'
ConstDeclAST::ConstDeclAST() {}
ConstDeclAST::~ConstDeclAST() {}

void ConstDeclAST::Dump_IR(char* IR, int last_sentence = 0) {
	constdefvec->Dump_IR(IR, last_sentence);
}

// ConstDefVec::= ConstDefVec ',' ConstDef | ConstDef
ConstDefVecAST::ConstDefVecAST() {}
ConstDefVecAST::~ConstDefVecAST() {}

void ConstDefVecAST::Dump_IR(char* IR, int last_sentence = 0) {
	for (int i = 0; i < itemvec.size(); i++) {
		itemvec[i]->Dump_IR(IR, last_sentence);
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

void ConstDefAST::Dump_IR(char* IR, int last_sentence = 0) {
	int const_value = calculate();
	now_table->insert(ident, ConstVar, const_value);
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


void LValAST::Dump_IR(char* IR, int last_sentence = 0) {
	Varient* v = now_table->search_until_root(ident);
	if (v->tag == ConstVar) {
		IRV.set_value(Integer, v->value);
	}
	else if (v->tag == Var) { //load 到寄存器
		IRV.set_value(Register, start_point);
		start_point += 1;
		std::string temp_IR = "  " + IRV.get_IR_value() + " = load " + now_table->search_until_root(ident)->get_str_value() + "\n";
		strcat(IR, const_cast<char*>(temp_IR.c_str()));
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

void VarDeclAST::Dump_IR(char* IR, int last_sentence = 0) {
	vardefvec->Dump_IR(IR, last_sentence);
}

// VarDefVec::=VarDefVec ',' VarDef | VarDef
VarDefVecAST::VarDefVecAST() {}
VarDefVecAST::~VarDefVecAST() {}


void VarDefVecAST::Dump_IR(char* IR, int last_sentence = 0) {
	for (int i = 0; i < itemvec.size(); ++i) {
		itemvec[i]->Dump_IR(IR, last_sentence);
	}
}

// VarDef::=IDENT|IDENT '=' InitVal 插入一个新的变量到当层符号表中，要查询目前整棵树的符号表情况，决定变量的标号
VarDefAST::VarDefAST() {}
VarDefAST::~VarDefAST() {}

void VarDefAST::Dump_IR(char* IR, int last_sentence = 0) {
	now_table->insert(ident, Var, 0);
	Varient* temp_var = now_table->search_until_root(ident);
	std::string str_value = temp_var->get_str_value();
	std::string temp_IR = "  " + str_value + " = alloc i32\n";
	strcat(IR, const_cast<char*>(temp_IR.c_str()));
	if (flag == 1) {
		initval->Dump_IR(IR, last_sentence);
		IRV = initval->IRV;
		temp_IR = "  store " + IRV.get_IR_value() + ", " + str_value + "\n";
		strcat(IR, const_cast<char*>(temp_IR.c_str()));
	}
}

// InitVal::=Exp
InitValAST::InitValAST() {}
InitValAST::~InitValAST() {}

void InitValAST::Dump_IR(char* IR, int last_sentence = 0) {
	exp->Dump_IR(IR, last_sentence);
	IRV = exp->IRV;
}



// Stmt::=MatchedStmt|OpenStmt
StmtAST::StmtAST() {}
StmtAST::~StmtAST() {}

void StmtAST::Dump_IR(char* IR, int last_sentence = 0) {
	if (now_block == NULL) {
		now_block = get_new_bb();
		now_block->output_into_block(IR);
	}
	if (flag == 0) {
		matchedstmt->Dump_IR(IR, last_sentence);
	}
	else if (flag == 1) {
		openstmt->Dump_IR(IR, last_sentence);
	}
}

//MatchedStmtAST:: IF (Exp) MatchedStmt else MatchedStmt | OtherStmt
MatchedStmtAST::MatchedStmtAST() {}
MatchedStmtAST::~MatchedStmtAST() {}

void MatchedStmtAST::Dump_IR(char* IR, int last_sentence = 0) {
	if (now_block == NULL) {
		now_block = get_new_bb();
		now_block->output_into_block(IR);
	}
	if (flag == 0) {
		Basic_Block* block_end = now_block->left;
		if (last_sentence == false) {
			block_end = new Basic_Block(now_block->left, NULL,now_block->loop_start,now_block->loop_end);
			bb_stack.push_back(block_end);
		}
		Basic_Block* block_else = new Basic_Block(block_end, NULL,now_block->loop_start,now_block->loop_end);
		Basic_Block* block_if = new Basic_Block(block_end, NULL,now_block->loop_start,now_block->loop_end);
		// Basic_Block* block_exp = new Basic_Block(block_if, block_else);
		bb_stack.push_back(block_else);
		bb_stack.push_back(block_if);
		// bb_stack.push_back(block_exp);
		// string temp_IR = "  jump " + block_exp->get_block_name() + "\n";//表示当前的basic block结束
		// strcat(IR, const_cast<char*>(temp_IR.c_str()));

		//现在写的逻辑是不短路求值，因此exp仍然在有原来的dump范围内 顺序应该是当前block先结束，然后exp开始dump，并依次进入新的basic block
		exp->Dump_IR(IR, last_sentence);
		string temp_IR = "  br " + exp->IRV.get_IR_value() + ", " + block_if->get_block_name() + ", " + block_else->get_block_name() + "\n\n";
		strcat(IR, const_cast<char*>(temp_IR.c_str()));
		if (now_block) {
			delete now_block;
			now_block = NULL;
		}
		matchedstmt1->Dump_IR(IR, true);
		if (now_block) {
			if (now_block->dead) {
				delete now_block;
				now_block = NULL;
			}
			else if (now_block->dead == false) {
				temp_IR = "  jump " + block_end->get_block_name() + "\n\n";
				strcat(IR, const_cast<char*>(temp_IR.c_str()));
				delete now_block;
				now_block = NULL;
			}
		}
		matchedstmt2->Dump_IR(IR, true);
		if (now_block) {
			if (now_block->dead) {
				delete now_block;
				now_block = NULL;
			}
			else if (now_block->dead == false) {
				temp_IR = "  jump " + block_end->get_block_name() + "\n\n";
				strcat(IR, const_cast<char*>(temp_IR.c_str()));
				delete now_block;
				now_block = NULL;
			}
		}
	}

	else if (flag == 1) {
		otherstmt->Dump_IR(IR, last_sentence);
	}
}

//OpenStmt IF (exp) matchedstmt ELSE openstmt | if (Exp) Stmt
OpenStmtAST::OpenStmtAST() {}
OpenStmtAST::~OpenStmtAST() {}
// 在dump中新增几个basic block意味着dump之后就要删除几个
void OpenStmtAST::Dump_IR(char* IR, int last_sentence = 0) {
	if (now_block == NULL) {
		now_block = get_new_bb();
		now_block->output_into_block(IR);
	}
	if (flag == 0) {
		Basic_Block* block_end = now_block->left;
		if (last_sentence == false) {
			block_end = new Basic_Block(now_block->left, NULL,now_block->loop_start,now_block->loop_end);
			bb_stack.push_back(block_end);
		}
		Basic_Block* block_else = new Basic_Block(block_end, NULL,now_block->loop_start,now_block->loop_end);
		bb_stack.push_back(block_else);
		Basic_Block* block_if = new Basic_Block(block_end, NULL,now_block->loop_start,now_block->loop_end);
		bb_stack.push_back(block_if);
		// Basic_Block* block_exp = new Basic_Block(block_if, block_else);
		exp->Dump_IR(IR, last_sentence);

		string temp_IR = "  br " + exp->IRV.get_IR_value() + ", " + block_if->get_block_name() + ", " + block_else->get_block_name() + "\n\n";
		strcat(IR, const_cast<char*>(temp_IR.c_str()));
		if (now_block) {
			delete now_block;
			now_block = NULL;
		}
		matchedstmt->Dump_IR(IR, true);
		if (now_block) {
			if (now_block->dead) {
				delete now_block;
				now_block = NULL;
			}
			else if (now_block->dead == false) {
				temp_IR = "  jump " + block_end->get_block_name() + "\n\n";
				strcat(IR, const_cast<char*>(temp_IR.c_str()));
				delete now_block;
				now_block = NULL;
			}
		}
		openstmt->Dump_IR(IR, true);
		if (now_block) {
			if (now_block->dead) {
				delete now_block;
				now_block = NULL;
			}
			else if (now_block->dead == false) {
				temp_IR = "  jump " + block_end->get_block_name() + "\n\n";
				strcat(IR, const_cast<char*>(temp_IR.c_str()));
				delete now_block;
				now_block = NULL;
			}
		}
	}
	else if (flag == 1) {
		Basic_Block* block_end = now_block->left;
		if (last_sentence == false) {
			block_end = new Basic_Block(now_block->left, NULL,now_block->loop_start,now_block->loop_end);
			bb_stack.push_back(block_end);
		}
		Basic_Block* block_if = new Basic_Block(block_end, NULL,now_block->loop_start,now_block->loop_end);
		bb_stack.push_back(block_if);
		exp->Dump_IR(IR, last_sentence);
		string temp_IR = "  br " + exp->IRV.get_IR_value() + ", " + block_if->get_block_name() + ", " + block_end->get_block_name() + "\n\n";
		strcat(IR, const_cast<char*>(temp_IR.c_str()));
		if (now_block) {//直接delete，不需要设定为dead
			delete now_block;
			now_block = NULL;
		}
		stmt->Dump_IR(IR, true);
		if (now_block) {
			if (now_block->dead) {
				delete now_block;
				now_block = NULL;
			}
			else if (now_block->dead == false) {
				temp_IR = "  jump " + block_end->get_block_name() + "\n\n";
				strcat(IR, const_cast<char*>(temp_IR.c_str()));
				delete now_block;
				now_block = NULL;
			}
		}
	}
}

// OtherStmtAST::=  Lval '=' Exp ';' | ExpExist ';' | Block | "return" ExpExist ';'|while ( Exp ) Stmt |continue; |break;
OtherStmtAST::OtherStmtAST() {}
OtherStmtAST::~OtherStmtAST() {}

void OtherStmtAST::Dump_IR(char* IR, int last_sentence = 0) {
	switch (flag)
	{
	case 0: {
		exp->Dump_IR(IR, last_sentence);
		IRV = exp->IRV;
		std::string key = dynamic_cast<LValAST&>(*lval).ident;
		std::string temp_IR = "  store " + IRV.get_IR_value() + ", " + (now_table->search_until_root(key))->get_str_value() + "\n";
		strcat(IR, const_cast<char*>(temp_IR.c_str()));
		break;
	}
	case 1: {
		expexist->Dump_IR(IR, last_sentence);
		IRV = expexist->IRV;
		break;
	}
	case 2: {
		std::shared_ptr<SymbolTable> son_table = std::shared_ptr<SymbolTable>(new SymbolTable(now_table));
		assert(son_table->pre_table == now_table);
		now_table = son_table;
		block->Dump_IR(IR, last_sentence);
		now_table = son_table->pre_table;
		break;
	}
	case 3: {
		if (expexist->flag == 0) {
			expexist->Dump_IR(IR, last_sentence);
			IRV = expexist->IRV;
			std::string temp_IR = "  ret " + IRV.get_IR_value() + "\n\n";
			strcat(IR, const_cast<char*>(temp_IR.c_str()));
			now_block->dead = true;
		}
		else if (expexist->flag == 1) {
			strcat(IR, "  ret \n\n");
			now_block->dead = true;
		}
		break;
	}
	case 4: {
		Basic_Block* block_end = now_block->left;
		if (last_sentence == false) {
			block_end = new Basic_Block(now_block->left, NULL,now_block->loop_start,now_block->loop_end);
			bb_stack.push_back(block_end);
		}
		Basic_Block* block_while = new Basic_Block(block_end, NULL,"",block_end->get_block_name());//回到的是当前的循环层,loop_end设定为同层
		Basic_Block* block_loop_start = new Basic_Block(block_end, NULL,now_block->loop_start,now_block->loop_end);
		block_while->loop_start = block_loop_start->get_block_name();
		bb_stack.push_back(block_while);
		bb_stack.push_back(block_loop_start);
		std::string temp_IR = "  jump " + block_loop_start->get_block_name() + "\n\n";//now block的最后一句
		strcat(IR, const_cast<char*>(temp_IR.c_str()));
		if (now_block) {
			delete now_block;
			now_block = NULL;
		}
		exp->Dump_IR(IR, true);
		if (last_sentence==true&&now_block->loop_start!=""){
			temp_IR = "  br " + exp->IRV.get_IR_value() + ", " + block_while->get_block_name() + ", " + now_block->loop_start + "\n\n";
			strcat(IR, const_cast<char*>(temp_IR.c_str()));
		}
		else {
			temp_IR = "  br " + exp->IRV.get_IR_value() + ", " + block_while->get_block_name() + ", " + block_end->get_block_name() + "\n\n";
			strcat(IR, const_cast<char*>(temp_IR.c_str()));
		}
		if (now_block) {
			delete now_block;
			now_block = NULL;
		}
		stmt->Dump_IR(IR, true);
		if (now_block) {
			if (now_block->dead) {//同样是可能有basic block内部ret的情况
				delete now_block;
				now_block = NULL;
			}
			else if (now_block->dead == false) {
				temp_IR = "  jump " + now_block->loop_start + "\n\n";
				strcat(IR, const_cast<char*>(temp_IR.c_str()));
				delete now_block;
				now_block = NULL;
			}
		}
		break;

	}
	
	case 5:{
		now_block->dead = true;
		std::string temp_IR = "  jump " + now_block->loop_start + "\n\n";
		strcat(IR, const_cast<char*>(temp_IR.c_str()));
		break;
	}
	case 6:{
		now_block->dead = true;
		std::string temp_IR = "  jump " + now_block->loop_end + "\n\n";
		strcat(IR, const_cast<char*>(temp_IR.c_str()));
		break;
	}
	}
}


//ExpExist ::= Exp|ε
ExpExistAST::ExpExistAST() {}
ExpExistAST::~ExpExistAST() {}


void ExpExistAST::Dump_IR(char* IR, int last_sentence = 0) {
	if (flag == 0) {
		exp->Dump_IR(IR, last_sentence);
		IRV = exp->IRV;
	}
}

// Exp ::= LOrExp
ExpAST::ExpAST() {}
ExpAST::~ExpAST() {}
int ExpAST::calculate() {
	return lorexp->calculate();
}

void ExpAST::Dump_IR(char* IR, int last_sentence = 0) {
	//有可能一个新的basic block以exp开头
	if (flag == 0) {
		if (now_block == NULL) {
			now_block = get_new_bb();//并不入栈
			now_block->output_into_block(IR);
		}
		lorexp->Dump_IR(IR, last_sentence);
		IRV = lorexp->IRV;
	}
}



// PrimaryExp ::= "(" Exp ")" | Number| LVal
PrimaryExpAST::PrimaryExpAST() {}
PrimaryExpAST::~PrimaryExpAST() {}
int PrimaryExpAST::calculate() {
	if (flag == 0) return exp->calculate();
	if (flag == 1) return number;
	if (flag == 2) return now_table->search_until_root(dynamic_cast<LValAST&>(*lval).ident)->value;
	return 1234567;
}

void PrimaryExpAST::Dump_IR(char* IR, int last_sentence = 0) {

	switch (flag) {
	case 0: {
		exp->Dump_IR(IR, last_sentence);
		IRV = exp->IRV;
		break;
	}
	case 1: {
		IRV.set_value(Integer, number);//不改变start point
		break;
	}
	case 2: {
		lval->Dump_IR(IR, last_sentence);
		IRV = lval->IRV;
		break;
	}
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
	return 1234567;
}
void UnaryExpAST::Dump_IR(char* IR, int last_sentence = 0) {
	// Set_IRV(); // 总是要先set irv
	if (flag == 0) {
		primaryexp->Dump_IR(IR, last_sentence);
		IRV = primaryexp->IRV;
	}
	else if (flag == 1) {
		unaryexp->Dump_IR(IR, last_sentence);
		// "+" 不处理
		if (unaryop->flag == 0) {
			IRV = unaryexp->IRV;
		}
		else {
			std::string temp_IR = "  ";
			IRV.set_value(Register, start_point);
			start_point += 1;
			temp_IR += IRV.get_IR_value() + " = ";
			// "-" 分类处理
			if (unaryop->flag == 1) {
				temp_IR += "sub 0, " + unaryexp->IRV.get_IR_value() + "\n";
			}
			// "!" 单独处理
			else if (unaryop->flag == 2) {
				temp_IR += "eq " + unaryexp->IRV.get_IR_value() + ", 0\n";
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
	return 1234567;
}

void MulExpAST::Dump_IR(char* IR, int last_sentence = 0) {
	if (flag == 0) {
		unaryexp->Dump_IR(IR, last_sentence);
		IRV = unaryexp->IRV;
	}
	else if (flag == 1) {
		mulexp->Dump_IR(IR, last_sentence);
		unaryexp->Dump_IR(IR, last_sentence);
		IRV.set_value(Register, start_point);
		start_point += 1;
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
	return 1234567;
}

void AddExpAST::Dump_IR(char* IR, int last_sentence = 0) {
	if (flag == 0) {
		mulexp->Dump_IR(IR, last_sentence);
		IRV = mulexp->IRV;
	}
	else if (flag == 1) {
		addexp->Dump_IR(IR, last_sentence);
		mulexp->Dump_IR(IR, last_sentence);
		IRV.set_value(Register, start_point);
		start_point += 1;
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
	return 1234567;
}

void RelExpAST::Dump_IR(char* IR, int last_sentence = 0) {
	if (flag == 0) {
		addexp->Dump_IR(IR, last_sentence);
		IRV = addexp->IRV;
	}
	else if (flag == 1) {
		relexp->Dump_IR(IR, last_sentence);
		addexp->Dump_IR(IR, last_sentence);
		IRV.set_value(Register, start_point);
		start_point += 1;
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
	return 1234567;
}

void EqExpAST::Dump_IR(char* IR, int last_sentence = 0) {
	if (flag == 0) {
		relexp->Dump_IR(IR, last_sentence);
		IRV = relexp->IRV;
	}
	else if (flag == 1) {
		eqexp->Dump_IR(IR, last_sentence);
		relexp->Dump_IR(IR, last_sentence);
		IRV.set_value(Register, start_point);
		start_point += 1;
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
	return 1234567;
}


void LAndExpAST::Dump_IR(char* IR, int last_sentence = 0) {
	if (flag == 0) {
		eqexp->Dump_IR(IR, last_sentence);
		IRV = eqexp->IRV;
	}
	else if (flag == 1) {
		landexp->Dump_IR(IR, last_sentence);
		eqexp->Dump_IR(IR, last_sentence);
		start_point += 2;//多用两个寄存器
		IRV.set_value(Register, start_point);
		start_point += 1;
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
	return 1234567;
}

void LOrExpAST::Dump_IR(char* IR, int last_sentence = 0) {
	if (flag == 0) {
		landexp->Dump_IR(IR, last_sentence);
		IRV = landexp->IRV;
	}
	else if (flag == 1) {
		lorexp->Dump_IR(IR, last_sentence);
		landexp->Dump_IR(IR, last_sentence);
		start_point += 1;//要多用一个寄存器
		IRV.set_value(Register, start_point);
		start_point += 1;
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
void UnaryOpAST::Dump_IR(char* IR, int last_sentence = 0) { assert(false); }// UnaryOp ::= "+" | "-" | "!";
void AddOpAST::Dump_IR(char* IR, int last_sentence = 0) { assert(false); }// AddOp ::= "+" | "-"
void MulOpAST::Dump_IR(char* IR, int last_sentence = 0) { assert(false); }// MulOp ::= "*" | "/" | "%"
void RelOpAST::Dump_IR(char* IR, int last_sentence = 0) { assert(false); }// ("<" | ">" | "<=" | ">=")
void EqOpAST::Dump_IR(char* IR, int last_sentence = 0) { assert(false); }//("==" | "!=")

