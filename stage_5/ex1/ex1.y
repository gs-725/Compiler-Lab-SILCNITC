%{
    #include <stdlib.h>
    #include <stdio.h>
    #include "ex1.h"
    #include "ex1.c"

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
%type <ast> ID_ NUM_ TEXT_ id l_value
%type <ast> expr stringExpr
%type <param> ParamList Param
%type <type> Type ReturnType 
%token PLUS_ MINUS_ STAR_ DIV_ MOD_ LT_ GT_ LE_ GE_ NE_ EQ_ AND_ OR_ AMPERSAND_
%token BEGIN_ END_ READ_ WRITE_ IF_ THEN_ ELSE_ ENDIF_ WHILE_ DO_ ENDWHILE_ BREAK_ CONTINUE_ REPEAT_ UNTIL_
%token MAIN_ RETURN_
%token DECL_ ENDDECL_
%token INT_ STR_
%token ID_ NUM_ TEXT_
%left LT_ GT_ LE_ GE_ NE_ EQ_ AND_ OR_
%left PLUS_ MINUS_
%left STAR_ DIV_ MOD_
%right UMINUS USTAR UAMP

%start Program

%%
//---------------------------------RULES---------------------------------------------

//---------------------------------PROGRAM--------------------------------------------
Program: GDeclBlock FnDefBlock MainBlock {
            $$ = NULL;
            printf("Parsing Successful\n");
            //GSTPrint();
            
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
// Assuming main must return int
MainBlock: INT_ MAIN_ '(' ')' '{' LDeclBlock Body '}' {
            if ($7->right->type != INTEGER) {
                printf("Line %d: Return type of main function doesn't match, expected int.\n", yylineno);
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
        
GDecl: ReturnType GIdList ';' {ASTChangeTypeGST($2, $1);} // ReturnType handles int, str, int*, str*

GIdList: GIdList ',' GId {$$ = makeNode(CONNECTOR, VOID, $1, NULL, $3, NULL, "GIDLIST");}
       | GId

GId: ID_ { // Variable/Array/Function - Type determined by ReturnType
        // If ReturnType was a pointer (e.g., int*), this ID will become a pointer variable.
        GSTInstall($1->name, VOID, 1, VARIABLE, NULL);
        $$ = $1;
    }
    | ID_ '[' NUM_ ']' { // Array
        GSTInstall($1->name, VOID, $3->val, ARRAY, NULL);
        $$ = $1;
    }
    /* AMBIGUOUS RULE REMOVED. Pointer variable declarations must be handled 
       when ReturnType is reduced from Type STAR_. 
    | STAR_ ID_ { // Pointer declaration: *p 
        GSTInstall($2->name, PVOID, 1, POINTER, NULL);
        $$ = $2;
    } */
    | ID_ '(' ParamList ')' { // Function declaration with parameters
        GSTInstall($1->name, VOID, 1, FUNCTION, ParamCopy(Phead));
        Phead = ParamDelete(Phead);
        $$ = $1;
    }
    | ID_ '(' ')' { // Function declaration without parameters
        GSTInstall($1->name, VOID, 1, FUNCTION, NULL);
        $$ = $1;
    }
    
    
//--------------------------------FUNCTIONS---------------------------------------------
FnDefBlock: FnDefBlock FnDef {$$ = makeNode(CONNECTOR, VOID, $1, NULL, $2, NULL, "FNDEFBLOCK");}
          | FnDef

// Functions using ReturnType
FnDef: ReturnType ID_ '(' ParamList ')' '{' LDeclBlock Body '}' {
            struct GST_Node *f = GSTLookup($2->name);
            // Error checks
            if (f == NULL) {
                printf("Line %d: Function \"%s\" not declared\n", yylineno, $2->name);
                exit(1);
            }
            if (f->type != $1) {
                printf("Line %d: Invalid return type of function \"%s\"\n", yylineno, $2->name);
                exit(1);
            }
            if (ParamCheck(f->Phead, Phead) == 0) {
                printf("Line %d: Parameter list of function \"%s\" doesn't match\n", yylineno, $2->name);
                exit(1);
            }
            if ($8->right->type != $1) {
                printf("Line %d: Return type of function \"%s\" doesn't match (Expected %d, Got %d)\n", yylineno, $2->name, $1, $8->right->type);
                exit(1);
            }
            // LST handling
            f->LST = LSTCopy(lst);
            lst = LSTDelete(lst);
            $$ = makeNode(FUNCTION, $1, $8, NULL, ParamToArg(Phead), f, f->name);
            Phead = ParamDelete(Phead);
            //ASTPrint($$);
            //printf("\n");
        }
        | ReturnType ID_ '(' ')' '{' LDeclBlock Body '}' {
            struct GST_Node *f = GSTLookup($2->name);
            // Error checks
            if (f == NULL) {
                printf("Line %d: Function \"%s\" not declared\n", yylineno, $2->name);
                exit(1);
            }
            if (f->type != $1) {
                printf("Line %d: Invalid return type of function \"%s\"\n", yylineno, $2->name);
                exit(1);
            }
            if (ParamCheck(f->Phead, NULL) == 0) {
                printf("Line %d: Parameter list of function \"%s\" doesn't match\n", yylineno, $2->name);
                exit(1);
            }
            if ($7->right->type != $1) {
                printf("Line %d: Return type of function \"%s\" doesn't match (Expected %d, Got %d)\n", yylineno, $2->name, $1, $7->right->type);
                exit(1);
            }
            // LST handling
            f->LST = LSTCopy(lst);
            lst = LSTDelete(lst);
            $$ = makeNode(FUNCTION, $1, $7, NULL, NULL, f, f->name);
            //ASTPrint($$);
            //printf("\n");
        }


//--------------------------------PARAMETERS---------------------------------------------
ParamList: ParamList ',' Param
         | Param

// Added support for pointer parameters: Type STAR_ ID_
Param: Type STAR_ ID_ { 
        Type p_type;
        if ($1 == INTEGER) p_type = POINTER_TO_INTEGER;
        else if ($1 == STRING) p_type = POINTER_TO_STRING;
        else {
            printf("Line %d: Invalid pointer parameter type.\n", yylineno);
            exit(1);
        }
        Phead = ParamInstall(Phead, $3->name, p_type, POINTER);
       }
     | Type ID_ {Phead = ParamInstall(Phead, $2->name, $1, VARIABLE);}


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

// LIdList: Handles comma-separated list of variables/pointers
LIdList: LIdList ',' ID_ {
            lst = LSTInstall(lst, $3->name, 1);
            $$ = makeNode(CONNECTOR, VOID, $1, NULL, $3, NULL, "LIDLIST");
        }
        | LIdList ',' STAR_ ID_ { // Pointer declaration: *p
            lst = LSTInstall(lst, $4->name, PVOID);
            $$ = makeNode(CONNECTOR, VOID, $1, NULL, $4, NULL, "LIDLIST");
        }
        | ID_ {
            lst = LSTInstall(lst, $1->name, 1);
            $$ = $1;
        }
        | STAR_ ID_ { // Pointer declaration: *p
            lst = LSTInstall(lst, $2->name, PVOID);
            $$ = $2;
        }


//--------------------------------ARGUMENTS---------------------------------------------
ArgList: ArgList ',' stringExpr {$$ = ASTArgAppend($1, $3);}
       | stringExpr


//--------------------------------TYPE---------------------------------------------
Type: INT_ {$$ = INTEGER;}
    | STR_ {$$ = STRING;}

// NEW: ReturnType can be a basic Type or a pointer Type
ReturnType: Type STAR_ {
            if ($1 == INTEGER) $$ = POINTER_TO_INTEGER;
            else if ($1 == STRING) $$ = POINTER_TO_STRING;
            else {
                printf("Line %d: Cannot define a pointer return type for this type.\n", yylineno);
                exit(1);
            }
          }
          | Type {$$ = $1;}

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

InputStmt: READ_ '(' l_value ')' { $$ = makeNode(READ, VOID, $3, NULL, NULL, NULL, "READ");}

OutputStmt: WRITE_ '(' stringExpr ')' { $$ = makeNode(WRITE, VOID, $3, NULL, NULL, NULL, "WRITE");}

AsgnStmt: l_value '=' stringExpr { $$ = makeNode(OPERATOR, INTEGER, $1, NULL, $3, NULL, "="); }

IfStmt: IF_ '(' expr ')' THEN_ Stmt_list ELSE_ Stmt_list ENDIF_ { $$ = makeNode(IF, VOID, $3, $6, $8, NULL, "IF");}
      | IF_ '(' expr ')' THEN_ Stmt_list ENDIF_ { $$ = makeNode(IF, VOID, $3, $6, NULL, NULL, "IF");}

WhileStmt: WHILE_ '(' expr ')' DO_ Stmt_list ENDWHILE_ { $$ = makeNode(WHILE, VOID, $3, NULL, $6, NULL, "WHILE");}

//---------------------------------EXPRESSIONS---------------------------------------------

expr : expr PLUS_ expr        {$$ = makeNode(OPERATOR, INTEGER, $1, NULL, $3, NULL, "+");}
     | expr MINUS_ expr        {$$ = makeNode(OPERATOR, INTEGER, $1, NULL, $3, NULL, "-");}
     | expr STAR_ expr        {$$ = makeNode(OPERATOR, INTEGER, $1, NULL, $3, NULL, "*");}
     | expr DIV_ expr        {$$ = makeNode(OPERATOR, INTEGER, $1, NULL, $3, NULL, "/");}
     | expr MOD_ expr        {$$ = makeNode(OPERATOR, INTEGER, $1, NULL, $3, NULL, "%");}
     | expr LT_ expr           {$$ = makeNode(OPERATOR, BOOLEAN, $1, NULL, $3, NULL, "<");}
     | expr GT_ expr           {$$ = makeNode(OPERATOR, BOOLEAN, $1, NULL, $3, NULL, ">");}
     | expr LE_ expr           {$$ = makeNode(OPERATOR, BOOLEAN, $1, NULL, $3, NULL, "<=");}
     | expr GE_ expr           {$$ = makeNode(OPERATOR, BOOLEAN, $1, NULL, $3, NULL, ">=");}
     | expr NE_ expr           {$$ = makeNode(OPERATOR, BOOLEAN, $1, NULL, $3, NULL, "!=");}
     | expr EQ_ expr           {$$ = makeNode(OPERATOR, BOOLEAN, $1, NULL, $3, NULL, "==");}
     | expr AND_ expr        {$$ = makeNode(OPERATOR, BOOLEAN, $1, NULL, $3, NULL, "&&");}
     | expr OR_ expr           {$$ = makeNode(OPERATOR, BOOLEAN, $1, NULL, $3, NULL, "||");}
     | '(' expr ')'            {$$ = $2;}
     | id                      {$$ = $1;}
     | NUM_                    {$$ = $1;}
     | STAR_ expr %prec USTAR { // Dereference: *expr (R-value)
             $$ = makePointerNode(POINTER, $2, "*");
         }
     | AMPERSAND_ id %prec UAMP { // Address-of: &id
             $$ = makePointerNode(ADDRESS, $2, "&");
        };

stringExpr: expr
          | TEXT_

l_value: id { $$ = $1; } 
       | STAR_ expr %prec USTAR { 
               $$ = makePointerNode(POINTER, $2, "*");
           };

id:    ID_ {
        $$ = $1;
        // Lookup in LST and then in GST
        struct LST_Node *curr_l = LSTLookup(lst, $1->name);
        if (curr_l) {
            $$->type = curr_l->type;
        } else {
            struct GST_Node *curr_g = GSTLookup($1->name);
            if (curr_g == NULL) {
                printf("Line %d: Variable \"%s\" not declared\n", yylineno, $1->name);
                exit(1);
            }
            $$->type = curr_g->type;
        }
    }
    | ID_ '[' expr ']' {
        // ... (Array access logic unchanged)
        if ($3->type == BOOLEAN) {
            printf("Line %d: Array index cannot be boolean\n", yylineno);
            exit(1);
        }

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
        // ... (Function call logic unchanged)
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
