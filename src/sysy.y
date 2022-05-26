%code requires {
  #include <memory>
  #include <string>
}

%{

#include <iostream>
#include <memory>
#include <string>
#include "ast.h"

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
// 修改参数类型的声明为一个BaseAST

%parse-param {std::unique_ptr<BaseAST> &ast } 


// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况


%union {
  BaseAST *ast_val;
  BlockItemVecAST *blockvecast_val;
  ConstDefVecAST *constvecast_val;
  std::string *str_val;
  int int_val;
}


// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
// 不指定类型, 不返回对应token值
%token INT RETURN PLUS MINUS SEQZ MUL DIV MOD LT GT LE GE EQ NEQ LOR LAND CONST
%token <str_val> IDENT
%token <int_val> INT_CONST

// 非终结符的类型定义 所有的非终结符都改成ast类型
%type <ast_val> FuncDef FuncType Block BlockItem BlockItemVec ConstDefVec
%type <ast_val> Stmt Exp PrimaryExp UnaryExp UnaryOp AddExp MulExp AddOp MulOp LOrExp LAndExp EqExp RelExp EqOp RelOp ConstExp 
%type <ast_val> Decl ConstDecl BType ConstDef ConstInitVal LVal VarDecl VardefVec Vardef InitVal
%type <int_val> Number //number我们这里是定义为int类型的

%%


// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// 之前我们定义了 FuncDef 会返回一个 str_val, 也就是字符串指针
// 而 parser 一旦解析完 CompUnit, 就说明所有的 token 都被解析了, 即解析结束了
// 此时我们应该把 FuncDef 返回的结果收集起来, 作为 AST 传给调用 parser 的函数
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值 

CompUnit
  : FuncDef 
  {
    auto comp_unit = make_unique<CompUnitAST>(); // 自动声明为一个compunitast的智能指针并且创建
    comp_unit->func_def = unique_ptr<BaseAST>($1);
    ast = move(comp_unit);
  };


// FuncDef ::= FuncType IDENT '(' ')' Block;
// 我们这里可以直接写 '(' 和 ')', 因为之前在 lexer 里已经处理了单个字符的情况
// 解析完成后, 把这些符号的结果收集起来, 然后拼成一个新的字符串, 作为结果返回
// $$ 表示非终结符的返回值, 我们可以通过给这个符号赋值的方法来返回结果
// 你可能会问, FuncType, IDENT 之类的结果已经是字符串指针了
// 为什么还要用 unique_ptr 接住它们, 然后再解引用, 把它们拼成另一个字符串指针呢
// 因为所有的字符串指针都是我们 new 出来的, new 出来的内存一定要 delete
// 否则会发生内存泄漏, 而 unique_ptr 这种智能指针可以自动帮我们 delete
// 虽然此处你看不出用 unique_ptr 和手动 delete 的区别, 但当我们定义了 AST 之后
// 这种写法会省下很多内存管理的负担
FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  };

// 同上, 不再解释
FuncType
  : INT 
  {
    $$  = new FuncTypeAST("int");
  };


Block
  :'{' '}'
  {
    auto block = new BlockAST();
    block->flag=0;
    $$ = block;
  }
  |'{' BlockItemVec '}' 
  {
    auto block = new BlockAST();
    block->flag=1;
    block->blockitemvec = unique_ptr<BaseAST>($2);
    $$ = block;
  };

BlockItemVec
  :BlockItemVec BlockItem 
  {
    auto blockitemvec = $1;
    dynamic_cast<BlockItemVecAST&>(*blockitemvec).itemvec.push_back(unique_ptr<BaseAST>($2));
    $$ = blockitemvec;
  }
  | BlockItem
  {
    auto blockitemvec = new BlockItemVecAST();
    blockitemvec->itemvec.push_back(unique_ptr<BaseAST>($1));
    $$ = blockitemvec;
  };

BlockItem
  :Decl
  {
    auto blockitem = new BlockItemAST();
    blockitem->decl = unique_ptr<BaseAST>($1);
    blockitem->flag=0;
    $$ = blockitem;
  }
  |Stmt
  {
    auto blockitem = new BlockItemAST();
    blockitem->stmt = unique_ptr<BaseAST>($1);
    blockitem->flag=1;
    $$ = blockitem;
  };

Decl 
  :ConstDecl
  {
    auto decl = new DeclAST();
    decl->constdecl = unique_ptr<BaseAST>($1);
    decl->flag=0;
    $$ = decl;
  };

ConstDecl //not sure
  : CONST BType ConstDefVec ';'
  {
    auto constdecl = new ConstDeclAST();
    constdecl->btype = unique_ptr<BaseAST>($2);
    constdecl->constdefvec = unique_ptr<BaseAST>($3);
    $$=constdecl;
  };


BType
  : INT {
    auto btype = new BtypeAST();
    btype->flag=0;
    $$ = btype;
  };

ConstDefVec
  :ConstDefVec ',' ConstDef
  {
    auto constdefvec = $1;
    dynamic_cast<ConstDefVecAST&>(*constdefvec).itemvec.push_back(unique_ptr<BaseAST>($3));
    $$ = constdefvec;
  }
  | ConstDef
  {
    auto constdefvec = new ConstDefVecAST();
    constdefvec->itemvec.push_back(unique_ptr<BaseAST>($1));
    $$ = constdefvec;
  };


ConstDef
  : IDENT '=' ConstInitVal
  {
    auto constdef = new ConstDefAST();
    constdef->ident = *unique_ptr<string>($1);
    constdef->constinitval = unique_ptr<BaseAST>($3);
    $$ = constdef;
  }
ConstInitVal
  : ConstExp
  {
    auto constinitval = new ConstInitValAST();
    constinitval->flag=0;
    constinitval->constexp = unique_ptr<BaseAST>($1);
    $$ = constinitval;
  };

LVal
  : IDENT
  {
    auto lval = new LValAST();
    lval->ident = *unique_ptr<string>($1);
    $$ = lval;
  };

ConstExp
  : Exp
  {
    auto constexp = new ConstExpAST();
    constexp->exp = unique_ptr<BaseAST>($1);
    constexp->flag = 0;
    $$ = constexp;
  };


Stmt
  : RETURN Exp ';' 
  {
    auto stmt = new StmtAST();
    stmt->flag = 0; // 表示return一个exp这种的分解方式
    stmt->exp = unique_ptr<BaseAST>($2);
    $$ = stmt;
  }
  | LVal '=' Exp ';'
  {
    auto stmt = new StmtAST();
    stmt->flag = 1;
    stmt->lval = unique_ptr<BaseAST>($1);
    stmt->exp = unique_ptr<BaseAST>($3);
    $$ = stmt;
  }
  ;


Exp 
  : LOrExp
  {
    auto exp = new ExpAST();
    exp->lorexp = unique_ptr<BaseAST>($1);
    exp->flag = 0; // 表示按照第一种方式分解
    $$ = exp;
  };

LOrExp
  : LAndExp
  {
    auto lorexp = new LOrExpAST();
    lorexp->landexp = unique_ptr<BaseAST>($1);
    lorexp->flag = 0;
    $$ = lorexp;
  }
  | LOrExp LOR LAndExp
  {
    auto lorexp = new LOrExpAST();
    lorexp->lorexp = unique_ptr<BaseAST>($1);
    lorexp->landexp = unique_ptr<BaseAST>($3);
    lorexp->flag = 1;
    $$ = lorexp;
  };

LAndExp
  : EqExp
  {
    auto landexp = new LAndExpAST();
    landexp->eqexp = unique_ptr<BaseAST>($1);
    landexp->flag = 0;
    $$ = landexp;
  }
  | LAndExp LAND EqExp
  {
    auto landexp = new LAndExpAST();
    landexp->landexp = unique_ptr<BaseAST>($1);
    landexp->eqexp = unique_ptr<BaseAST>($3);
    landexp->flag = 1;
    $$ = landexp;
  };

EqExp
  : RelExp
  {
    auto eqexp = new EqExpAST();
    eqexp->relexp = unique_ptr<BaseAST>($1);
    eqexp->flag = 0;
    $$ = eqexp;
  }
  | EqExp EqOp RelExp
  {
    auto eqexp = new EqExpAST();
    eqexp->eqexp = unique_ptr<BaseAST>($1);
    eqexp->eqop = unique_ptr<BaseAST>($2);
    eqexp->relexp = unique_ptr<BaseAST>($3);
    eqexp->flag = 1;
    $$ = eqexp;
  };

RelExp 
  : AddExp
  {
    auto relexp = new RelExpAST();
    relexp->addexp = unique_ptr<BaseAST>($1);
    relexp->flag = 0;
    $$ = relexp;
  }
  | RelExp RelOp AddExp
  {
    auto relexp = new RelExpAST();
    relexp->relexp = unique_ptr<BaseAST>($1);
    relexp->relop = unique_ptr<BaseAST>($2);
    relexp->addexp = unique_ptr<BaseAST>($3);
    relexp->flag = 1;
    $$ = relexp;
  };

AddExp
  : MulExp 
  {
    auto addexp = new AddExpAST();
    addexp->mulexp = unique_ptr<BaseAST>($1);
    addexp->flag =0;
    $$ = addexp;
  }
  | AddExp AddOp MulExp
  {
    auto addexp = new AddExpAST();
    addexp->addexp = unique_ptr<BaseAST>($1);
    addexp->addop = unique_ptr<BaseAST>($2);
    addexp->mulexp = unique_ptr<BaseAST>($3);
    addexp->flag = 1;
    $$ = addexp;
  };

MulExp 
  : UnaryExp 
  {
    auto mulexp = new MulExpAST();
    mulexp->unaryexp = unique_ptr<BaseAST>($1);
    mulexp->flag = 0; // 表示按照第一种方式分解
    $$ = mulexp;
  }
  | MulExp MulOp UnaryExp
  {
    auto mulexp = new MulExpAST();
    mulexp->mulexp = unique_ptr<BaseAST>($1);
    mulexp->mulop = unique_ptr<BaseAST>($2);
    mulexp->unaryexp = unique_ptr<BaseAST>($3);
    mulexp->flag = 1; // 表示按照第二种方式分解
    $$ = mulexp;
  };

PrimaryExp
  : '(' Exp ')'
  {
    auto primaryexp = new PrimaryExpAST();
    primaryexp->flag = 0;
    primaryexp->exp = unique_ptr<BaseAST>($2);
    $$ = primaryexp;

  }
  | Number
  {
    auto primaryexp = new PrimaryExpAST();
    primaryexp->flag = 1;
    primaryexp->number = int($1);
    $$ = primaryexp;
  }
  | LVal
  {
    auto primaryexp = new PrimaryExpAST();
    primaryexp->flag = 2;
    primaryexp->lval   = unique_ptr<BaseAST>($1);
    $$ = primaryexp;
  }

Number
  : INT_CONST 
  {
    $$ = int($1);
  };

UnaryExp
  : PrimaryExp
  {
    auto unaryexp = new UnaryExpAST();
    unaryexp->flag = 0;
    unaryexp->primaryexp = unique_ptr<BaseAST>($1);
    $$ = unaryexp;
  }
  | UnaryOp UnaryExp
  {
    auto unaryexp = new UnaryExpAST();
    unaryexp->flag = 1;
    unaryexp->unaryop = unique_ptr<BaseAST>($1);
    unaryexp->unaryexp = unique_ptr<BaseAST>($2);
    $$ = unaryexp;
  };

UnaryOp
  : PLUS{
    auto unaryop = new UnaryOpAST();
    unaryop->flag=0;
    $$ = unaryop;
  }
  | MINUS{
    auto unaryop = new UnaryOpAST();
    unaryop->flag=1;
    $$ = unaryop;
  }
  | SEQZ{
    auto unaryop = new UnaryOpAST();
    unaryop->flag=2;
    $$ = unaryop;
  };

AddOp
  : PLUS{
    auto addop = new AddOpAST();
    addop->flag = 0;
    $$ = addop;
  }
  | MINUS{
    auto addop = new AddOpAST();
    addop->flag = 1;
    $$ = addop;
  };

MulOp
  : MUL{
    auto mulop = new MulOpAST();
    mulop->flag = 0;
    $$ = mulop;
  }
  | DIV{
    auto mulop = new MulOpAST();
    mulop->flag = 1;
    $$ = mulop;
  }
  | MOD{
    auto mulop = new MulOpAST();
    mulop->flag = 2;
    $$ = mulop;
  };

EqOp
  : EQ{
    auto eqop = new EqOpAST();
    eqop->flag = 0;
    $$ = eqop;
  }
  | NEQ{
    auto eqop = new EqOpAST();
    eqop->flag = 1;
    $$ = eqop;
  };

RelOp
  : LT{
    auto relop = new RelOpAST();
    relop->flag = 0;
    $$ = relop;
  }
  | GT{
    auto relop = new RelOpAST();
    relop->flag = 1;
    $$ = relop;
  }
  | LE{
    auto relop = new RelOpAST();
    relop->flag = 2;
    $$ = relop;
  }
  | GE{
    auto relop = new RelOpAST();
    relop->flag = 3;
    $$ = relop;
  };




%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
