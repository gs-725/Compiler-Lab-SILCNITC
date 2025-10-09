%{
    #include <stdlib.h>
    #include <stdio.h>
    #include "t3.h"
    #include "t3.c"

    extern FILE* yyin;
    extern char* yytext;
    extern int yylineno;

    void yyerror(char const *s);
    int yylex(void);

    struct LSTable *lst;
    struct ParamNode *Phead;

    FILE *ft;
%}

%union{
    struct AST_Node *ast;
    struct ParamNode *param;
    int type;
}

%type <ast> Program
%type <ast> MainBlock Body
%type <ast> GDeclBlock GDeclList GDecl GIdList GId
%type <ast> FnDefBlock FnDef
%type <ast> LDeclBlock LDeclList LDecl LIdList
%type <ast> ArgList
%type <ast> Stmt_list Stmt InputStmt OutputStmt AsgnStmt IfStmt WhileStmt
%type <ast> ID_ NUM_ TEXT_ id
%type <ast> expr stringExpr
%type <param> ParamList Param
%type <type> Type
%token PLUS_ MINUS_ MUL_ DIV_ MOD_ LT_ GT_ LE_ GE_ NE_ EQ_ AND_ OR_
%token BEGIN_ END_ READ_ WRITE_ IF_ THEN_ ELSE_ ENDIF_ WHILE_ DO_ ENDWHILE_ BREAK_ CONTINUE_
%token MAIN_ RETURN_
%token DECL_ ENDDECL_
%token INT_ STR_
%token ID_ NUM_ TEXT_
%left LT_ GT_ LE_ GE_ NE_ EQ_ AND_ OR_
%left PLUS_ MINUS_
%left MUL_ DIV_ MOD_

%start Program

%%
//---------------------------------RULES---------------------------------------------

//---------------------------------PROGRAM--------------------------------------------
Program: GDeclBlock FnDefBlock MainBlock {
            $$ = NULL;
            printf("Parsing Successful\n");
            //GSTPrint();
            //localprint();
            generateHeader(ft);
            
           while (strcmp($2->s, "FNDEFBLOCK") == 0) {
                codeGen($2->right, $2->right->GSTentry->LST, ft);
                $2 = $2->left;
            }
            codeGen($2, $2->GSTentry->LST, ft);

            codeGen($3, $3->GSTentry->LST, ft);

            fclose(ft);
            
           
            exit(0);
        }
        | GDeclBlock MainBlock {
            $$ = NULL;
            printf("Parsing Successful\n");

            generateHeader(ft);

            fprintf(ft, "F%d:\n", $2->GSTentry->flabel);
            codeGen($2, $2->GSTentry->LST, ft);

            fclose(ft);
            exit(0);
        }
        | MainBlock {
            $$ = NULL;
            printf("Parsing Successful\n");

            generateHeader(ft);

            fprintf(ft, "F%d:\n", $1->GSTentry->flabel);
            codeGen($1, $1->GSTentry->LST, ft);

            fclose(ft);
            exit(0);
        }


//--------------------------------MAIN BLOCK---------------------------------------------
MainBlock: INT_ MAIN_ '(' ')' '{' LDeclBlock Body '}' {
            if ($7->right->type != INTEGER) {
                printf("Line %d: Return type of main function doesn't match\n", yylineno);
                exit(1);
            }
            GSTInstall("main", INTEGER, 1, FUNCTION, NULL);
            struct GST_Node *main = GSTLookup("main");
            main->LST = LSTCopy(lst);
            lst = LSTDelete(lst);
            $$ = makeNode(FUNCTION, INTEGER, $7, NULL, NULL, main, "main");
           	//print_tree($$, 0, 0);
            //printf("\n");
        }


//---------------------------------BODY---------------------------------------------
Body: BEGIN_ Stmt_list RETURN_ stringExpr ';' END_ {
        struct AST_Node *temp = makeNode(RET, $4->type, $4, NULL, NULL, NULL, "RETURN");
        $$ = makeNode(CONNECTOR, VOID, $2, NULL, temp, NULL, "BODY");
    }
    | BEGIN_ RETURN_ stringExpr ';' END_ {
        struct AST_Node *temp = makeNode(RET, $3->type, $3, NULL, NULL, NULL, "RETURN");
        $$ = makeNode(CONNECTOR, VOID, NULL, NULL, temp, NULL, "BODY");
    }


//---------------------------------GLOBAL DECLARATIONS--------------------------------
GDeclBlock: DECL_ GDeclList ENDDECL_ {$$ = $2; /*GSTPrint();*/}
          | DECL_ ENDDECL_ {$$ = NULL;}

GDeclList: GDeclList GDecl
         | GDecl
        
GDecl: Type GIdList ';' {ASTChangeTypeGST($2, $1);}

GIdList: GIdList ',' GId {$$ = makeNode(CONNECTOR, VOID, $1, NULL, $3, NULL, "GIDLIST");}
       | GId

GId: ID_ {
        GSTInstall($1->name, VOID, 1, VARIABLE, NULL);
        $$ = $1;
    }
    | ID_ '[' NUM_ ']' {
        GSTInstall($1->name, VOID, $3->val, ARRAY, NULL);
        $$ = $1;
    }
    | ID_ '(' ParamList ')' {
        GSTInstall($1->name, VOID, 1, FUNCTION, ParamCopy(Phead));
        Phead = ParamDelete(Phead);
        $$ = $1;
    }
    | ID_ '(' ')' {
        GSTInstall($1->name, VOID, 1, FUNCTION, NULL);
        $$ = $1;
    }


//--------------------------------FUNCTIONS---------------------------------------------
FnDefBlock: FnDefBlock FnDef {$$ = makeNode(CONNECTOR, VOID, $1, NULL, $2, NULL, "FNDEFBLOCK");}
          | FnDef

FnDef: Type ID_ '(' ParamList ')' '{' LDeclBlock Body '}' {
            struct GST_Node *f = GSTLookup($2->name);
            // Error if function not declared
            if (f == NULL) {
                printf("Line %d: Function \"%s\" not declared\n", yylineno, $2->name);
                exit(1);
            }
            // Error if return type doesn't match
            if (f->type != $1) {
                printf("Line %d: Invalid return type of function \"%s\"\n", yylineno, $2->name);
                exit(1);
            }
            // Error if parameter list doesn't match
            if (ParamCheck(f->Phead, Phead) == 0) {
                printf("Line %d: Parameter list of function \"%s\" doesn't match\n", yylineno, $2->name);
                exit(1);
            }
            // Error if return type of ast doesn't match
            if ($8->right->type != $1) {
                printf("Line %d: Return type of function \"%s\" doesn't match\n", yylineno, $2->name);
                exit(1);
            }
            // Add parameters to LST
            f->LST = LSTCopy(lst);
            lst = LSTDelete(lst);
            $$ = makeNode(FUNCTION, $1, $8, NULL, ParamToArg(Phead), f, f->name);
            Phead = ParamDelete(Phead);
            //ASTPrint($$);
            //printf("\n");
        }
        | Type ID_ '(' ')' '{' LDeclBlock Body '}' {
            struct GST_Node *f = GSTLookup($2->name);
            // Error if function not declared
            if (f == NULL) {
                printf("Line %d: Function \"%s\" not declared\n", yylineno, $2->name);
                exit(1);
            }
            // Error if return type doesn't match
            if (f->type != $1) {
                printf("Line %d: Invalid return type of function \"%s\"\n", yylineno, $2->name);
                exit(1);
            }
            // Error if parameter list doesn't match
            if (ParamCheck(f->Phead, NULL) == 0) {
                printf("Line %d: Parameter list of function \"%s\" doesn't match\n", yylineno, $2->name);
                exit(1);
            }
            // Error if return type of ast doesn't match
            if ($7->right->type != $1) {
                printf("Line %d: Return type of function \"%s\" doesn't match\n", yylineno, $2->name);
                exit(1);
            }
            f->LST = LSTCopy(lst);
            lst = LSTDelete(lst);
            $$ = makeNode(FUNCTION, $1, $7, NULL, NULL, f, f->name);
            //ASTPrint($$);
            //printf("\n");
        }


//--------------------------------PARAMETERS---------------------------------------------
ParamList: ParamList ',' Param
         | Param
        

Param: Type ID_ {Phead = ParamInstall(Phead, $2->name, $1, VARIABLE);}


//--------------------------------LOCAL DECLARATIONS-------------------------------------
LDeclBlock: DECL_ LDeclList ENDDECL_ {
                lst = LSTParamInstall(lst, Phead);
                //LSTPrint(lst);
                $$ = $2;
            }
            | DECL_ ENDDECL_ {
                lst = LSTParamInstall(lst, Phead);
                //LSTPrint(lst);
                $$ = NULL;
            }
            | %empty {
                lst = LSTParamInstall(lst, Phead);
                //LSTPrint(lst);
                $$ = NULL;
            }

LDeclList: LDeclList LDecl
         | LDecl

LDecl: Type LIdList ';' {ASTChangeTypeLST(lst, $2, $1);}

LIdList: LIdList ',' ID_ {
            lst = LSTInstall(lst, $3->name, 1);
            $$ = makeNode(CONNECTOR, VOID, $1, NULL, $3, NULL, "LIDLIST");
        }
        | ID_ {
            lst = LSTInstall(lst, $1->name, 1);
            $$ = $1;
        }


//--------------------------------ARGUMENTS---------------------------------------------
ArgList: ArgList ',' stringExpr {$$ = ASTArgAppend($1, $3);}
       | stringExpr


//--------------------------------TYPE---------------------------------------------
Type: INT_ {$$ = 0;}
    | STR_ {$$ = 2;}

//---------------------------------STATEMENTS---------------------------------------------

Stmt_list: Stmt_list Stmt ';' {$$ = makeNode(CONNECTOR, VOID, $1, NULL, $2, NULL, "CONNECTOR");}
    | Stmt ';' {$$ = $1;}

Stmt: InputStmt
    | OutputStmt
    | AsgnStmt
    | IfStmt
    | WhileStmt
    | BREAK_ { $$ = makeNode(BREAK, VOID, NULL, NULL, NULL, NULL, "BREAK"); }
    | CONTINUE_ { $$ = makeNode(CONTINUE, VOID, NULL, NULL, NULL, NULL, "CONTINUE"); }

InputStmt: READ_ '(' id ')' { $$ = makeNode(READ, VOID, $3, NULL, NULL, NULL, "READ");}

OutputStmt: WRITE_ '(' stringExpr ')' { $$ = makeNode(WRITE, VOID, $3, NULL, NULL, NULL, "WRITE");}

AsgnStmt: id '=' stringExpr { $$ = makeNode(OPERATOR, INTEGER, $1, NULL, $3, NULL, "="); }

IfStmt: IF_ '(' expr ')' THEN_ Stmt_list ELSE_ Stmt_list ENDIF_ { $$ = makeNode(IF, VOID, $3, $6, $8, NULL, "IF");}
    | IF_ '(' expr ')' THEN_ Stmt_list ENDIF_ { $$ = makeNode(IF, VOID, $3, $6, NULL, NULL, "IF");}

WhileStmt: WHILE_ '(' expr ')' DO_ Stmt_list ENDWHILE_ { $$ = makeNode(WHILE, VOID, $3, NULL, $6, NULL, "WHILE");}

//---------------------------------EXPRESSIONS---------------------------------------------

expr : expr PLUS_ expr		{$$ = makeNode(OPERATOR, INTEGER, $1, NULL, $3, NULL, "+");}
    | expr MINUS_ expr		{$$ = makeNode(OPERATOR, INTEGER, $1, NULL, $3, NULL, "-");}
    | expr MUL_ expr		{$$ = makeNode(OPERATOR, INTEGER, $1, NULL, $3, NULL, "*");}
    | expr DIV_ expr		{$$ = makeNode(OPERATOR, INTEGER, $1, NULL, $3, NULL, "/");}
    | expr MOD_ expr		{$$ = makeNode(OPERATOR, INTEGER, $1, NULL, $3, NULL, "%");}
    | expr LT_ expr			{$$ = makeNode(OPERATOR, BOOLEAN, $1, NULL, $3, NULL, "<");}
    | expr GT_ expr			{$$ = makeNode(OPERATOR, BOOLEAN, $1, NULL, $3, NULL, ">");}
    | expr LE_ expr			{$$ = makeNode(OPERATOR, BOOLEAN, $1, NULL, $3, NULL, "<=");}
    | expr GE_ expr			{$$ = makeNode(OPERATOR, BOOLEAN, $1, NULL, $3, NULL, ">=");}
    | expr NE_ expr			{$$ = makeNode(OPERATOR, BOOLEAN, $1, NULL, $3, NULL, "!=");}
    | expr EQ_ expr			{$$ = makeNode(OPERATOR, BOOLEAN, $1, NULL, $3, NULL, "==");}
    | expr AND_ expr		{$$ = makeNode(OPERATOR, BOOLEAN, $1, NULL, $3, NULL, "&&");}
    | expr OR_ expr			{$$ = makeNode(OPERATOR, BOOLEAN, $1, NULL, $3, NULL, "||");}
    | '(' expr ')' 			{$$ = $2;}
    | id					{$$ = $1;}
    | NUM_					{$$ = $1;}

stringExpr: expr
          | TEXT_


id:	ID_ {
        $$ = $1;
        // Lookup in LST and then in GST
        struct LST_Node *curr = LSTLookup(lst, $1->name);
        if (curr) {
            $$->type = curr->type;
        } else {
            struct GST_Node *curr = GSTLookup($1->name);
            if (curr == NULL) {
                printf("Line %d: Variable \"%s\" not declared\n", yylineno, $1->name);
                exit(1);
            }
            $$->type = curr->type;
        }
    }
    | ID_ '[' expr ']' {
        // Error if expr is boolean
        if ($3->type == BOOLEAN) {
            printf("Line %d: Array index cannot be boolean\n", yylineno);
            exit(1);
        }

        // Lookup in GST
        struct GST_Node *curr = GSTLookup($1->name);
        if (curr == NULL) {
            printf("Line %d: Array \"%s\" not declared\n", yylineno, $1->name);
            exit(1);
        }
        if (curr->typeofvar != ARRAY) {
            printf("Line %d: \"%s\" is not an array\n", yylineno, $1->name);
            exit(1);
        }
        $1->type = curr->type;
        $$ = makeArrayLeafNode($1->name, $3, "ARRAY");
    }
    | ID_ '(' ArgList ')' {
        struct GST_Node *curr = GSTLookup($1->name);
        if (curr == NULL) {
            printf("Line %d: Function \"%s\" not declared\n", yylineno, $1->name);
            exit(1);
        }
        if (curr->typeofvar != FUNCTION) {
            printf("Line %d: \"%s\" is not a function\n", yylineno, $1->name);
            exit(1);
        }
        if (checkASTParam(curr->Phead, $3) == 0) {
            printf("Line %d: Wrong arguments in \"%s\", does not match with declaration\n", yylineno, $1->name);
            exit(1);
        }
        $1->nodetype = FUNCTIONCALL;
        $1->type = curr->type;
        $1->arg_list = $3;
        $$ = $1;
    }
    | ID_ '(' ')' {
        struct GST_Node *curr = GSTLookup($1->name);
        if (curr == NULL) {
            printf("Line %d: Function \"%s\" not declared\n", yylineno, $1->name);
            exit(1);
        }
        if (curr->typeofvar != FUNCTION) {
            printf("Line %d: \"%s\" is not a function\n", yylineno, $1->name);
            exit(1);
        }

        // Ensure declared function has no parameters
        if (curr->Phead != NULL) {
            printf("Line %d: Function \"%s\" expects arguments, but called with none\n", yylineno, $1->name);
            exit(1);
        }

        $1->nodetype = FUNCTIONCALL;
        $1->type = curr->type;
        $1->arg_list = NULL;
        $$ = $1;
    };

%%

void yyerror(char const *s)
{
    printf("yyerror | Line: %d\n%s: %s\n", yylineno, s, yytext);
    exit(1);
}


int main(int argc, char *argv[]) 
{
    lst = LSTInitTable();
    Phead = NULL;
    char fname[256];
    scanf("%s",fname);
    yyin=fopen(fname,"r");
    ft = fopen("output.xsm", "w");
    yyparse();

    return 0;
}
