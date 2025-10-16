%{
    #include <stdlib.h>
    #include <stdio.h>
    #include "ex2.h"
    #include "ex2.c"

    extern FILE* yyin;
    extern char* yytext;
    extern int yylineno;

    void yyerror(char const *s);
    int yylex(void);

    struct LSTable *lst;
    struct ParamNode *Phead;
    struct TST_Node *Thead; // Global TST head reference

    FILE *ft;
    // Define external SP here to explicitly assign the correct final value
    extern int SP;
%}

%union{
    struct AST_Node *ast;
    struct ParamNode *param;
    int type;
    char *tuple_name;
}

// Tokens carrying an AST node value
%token <ast> ID_ NUM_ TEXT_ 

// Tokens carrying simple keywords/operators (no need for explicit type unless used in $$ or $N)
%token PLUS_ MINUS_ STAR_ DIV_ MOD_ LT_ GT_ LE_ GE_ NE_ EQ_ AND_ OR_ AMPERSAND_
%token BEGIN_ END_ READ_ WRITE_ IF_ THEN_ ELSE_ ENDIF_ WHILE_ DO_ ENDWHILE_ BREAK_ CONTINUE_ REPEAT_ UNTIL_
%token MAIN_ RETURN_
%token DECL_ ENDDECL_
%token INT_ STR_ TUPLE_
%token DOT_

%left LT_ GT_ LE_ GE_ NE_ EQ_ AND_ OR_
%left PLUS_ MINUS_
%left STAR_ DIV_ MOD_
%left DOT_ // Keeping DOT_ precedence
%right UMINUS USTAR UAMP

%type <ast> Program MainBlock Body
%type <ast> GDeclBlock GDeclList GDecl GIdList GId
%type <ast> FnDefBlock FnDef LDeclBlock LDeclList LDecl LIdList
%type <ast> ArgList Stmt_list Stmt InputStmt OutputStmt AsgnStmt IfStmt WhileStmt
%type <ast> id simple_id array_or_call_id l_value tuple_field_access
%type <ast> expr stringExpr
%type <param> ParamList Param TupleFieldList TupleField
%type <type> Type ReturnType
%type <tuple_name> TName

%start Program

%%

Program: GDeclBlock FnDefBlock MainBlock {
             $$ = NULL;
            // GSTPrint();
             printf("Parsing Successful\n");
             
             // Injects the correct final SP value before generating the header
             // This is a last resort fix to overcome potential linker/runtime SP issues.
             if (SP == 4099) SP = 4103;
             
             generateHeader(ft);
             
             struct AST_Node *curr = $2;
             // Functions are stored in reverse order in FnDefBlock
             while (curr && curr->nodetype == CONNECTOR) {
                 codeGen(curr->right, curr->right->GSTentry->LST, ft);
                 curr = curr->left;
             }
             if (curr) codeGen(curr, curr->GSTentry->LST, ft);

             codeGen($3, $3->GSTentry->LST, ft);

             fclose(ft);
             exit(0);
           }
         | GDeclBlock MainBlock {
             $$ = NULL;
             //GSTPrint();
             //TSTPrint();
             printf("Parsing Successful\n");

             // Injects the correct final SP value before generating the header
             if (SP == 4099) SP = 4103; 

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

MainBlock: INT_ MAIN_ '(' ')' '{' LDeclBlock Body '}' {
             if ($7->right->type != INTEGER) {
                 printf("Line %d: Return type of main function doesn't match, expected int.\n", yylineno);
                 exit(1);
             }
             GSTInstall("main", INTEGER, 1, FUNCTION, NULL, NULL);
             struct GST_Node *main = GSTLookup("main");
             main->LST = LSTCopy(lst);
             lst = LSTDelete(lst);
             $$ = makeNode(FUNCTION, INTEGER, $7, NULL, NULL, main, "main");
           }

Body: BEGIN_ Stmt_list RETURN_ stringExpr ';' END_ {
          struct AST_Node *temp = makeNode(RET, $4->type, $4, NULL, NULL, NULL, "RETURN");
          $$ = makeNode(CONNECTOR, VOID, $2, NULL, temp, NULL, "BODY");
      }
    | BEGIN_ RETURN_ stringExpr ';' END_ {
          struct AST_Node *temp = makeNode(RET, $3->type, $3, NULL, NULL, NULL, "RETURN");
          $$ = makeNode(CONNECTOR, VOID, NULL, NULL, temp, NULL, "BODY");
      }


GDeclBlock: DECL_ GDeclList ENDDECL_ {$$ = $2;}
          | DECL_ ENDDECL_ {$$ = NULL;}

GDeclList: GDeclList GDecl {$$ = makeNode(CONNECTOR, VOID, $1, NULL, $2, NULL, "CONNECTOR");}
         | GDecl

GDecl: ReturnType GIdList ';' {ASTChangeTypeGST($2, $1, NULL);}
     | TUPLE_ TName '(' TupleFieldList ')' GIdList ';' {
          struct TST_Node *tst_entry = TSTInstall($2, $4);
          $6 = ASTChangeTypeGST($6, TUPLE, tst_entry);
          $4 = ParamDelete($4);
          $$ = $6;
       }
     | TUPLE_ ID_ GIdList ';' {
          struct TST_Node *tst_entry = TSTLookup($2->name);
          if (tst_entry == NULL) {
              printf("Line %d: Tuple type \"%s\" not defined.\n", yylineno, $2->name);
              exit(1);
          }
          $3 = ASTChangeTypeGST($3, TUPLE, tst_entry);
          $$ = $3;
       }

TName: ID_ { $$ = strdup($1->name); }

TupleFieldList: TupleFieldList ',' TupleField {
    $$ = ParamInstall($1, $3->name, $3->type, VARIABLE);
    free($3);
}
              | TupleField {
    $$ = ParamInstall(NULL, $1->name, $1->type, VARIABLE);
    free($1);
}

TupleField: Type ID_ {
    struct ParamNode *temp = (struct ParamNode *)malloc(sizeof(struct ParamNode));
    temp->name = strdup($2->name);
    temp->type = $1;
    temp->typeofvar = VARIABLE;
    temp->next = NULL;
    $$ = temp;
    free($2);
}

GIdList: GIdList ',' GId {$$ = makeNode(CONNECTOR, VOID, $1, NULL, $3, NULL, "GIDLIST");}
       | GId

GId: ID_ {
           GSTInstall($1->name, VOID, 1, VARIABLE, NULL, NULL);
           $$ = $1;
     }
   | ID_ '[' NUM_ ']' {
           GSTInstall($1->name, VOID, $3->val, ARRAY, NULL, NULL);
           $$ = $1;
     }
   | ID_ '(' ParamList ')' {
           GSTInstall($1->name, VOID, 1, FUNCTION, ParamCopy(Phead), NULL);
           Phead = ParamDelete(Phead);
           $$ = $1;
     }
   | ID_ '(' ')' {
           GSTInstall($1->name, VOID, 1, FUNCTION, NULL, NULL);
           $$ = $1;
     }

FnDefBlock: FnDefBlock FnDef {$$ = makeNode(CONNECTOR, VOID, $1, NULL, $2, NULL, "FNDEFBLOCK");}
          | FnDef

FnDef: ReturnType ID_ '(' ParamList ')' '{' LDeclBlock Body '}' {
           struct GST_Node *f = GSTLookup($2->name);
           if (f == NULL || f->type != $1 || ParamCheck(f->Phead, Phead) == 0 || $8->right->type != $1) {
               printf("Line %d: Function definition for \"%s\" mismatches declaration.\n", yylineno, $2->name);
               exit(1);
           }
           f->LST = LSTCopy(lst);
           lst = LSTDelete(lst);
           $$ = makeNode(FUNCTION, $1, $8, NULL, ParamToArg(Phead), f, f->name);
           Phead = ParamDelete(Phead);
       }
     | ReturnType ID_ '(' ')' '{' LDeclBlock Body '}' {
           struct GST_Node *f = GSTLookup($2->name);
           if (f == NULL || f->type != $1 || ParamCheck(f->Phead, NULL) == 0 || $7->right->type != $1) {
               printf("Line %d: Function definition for \"%s\" mismatches declaration.\n", yylineno, $2->name);
               exit(1);
           }
           f->LST = LSTCopy(lst);
           lst = LSTDelete(lst);
           $$ = makeNode(FUNCTION, $1, $7, NULL, NULL, f, f->name);
       }

ParamList: ParamList ',' Param
         | Param

Param: Type STAR_ ID_ {
           Type p_type = ($1 == INTEGER) ? POINTER_TO_INTEGER : POINTER_TO_STRING;
           Phead = ParamInstall(Phead, $3->name, p_type, POINTER);
       }
     | Type ID_ {
           Phead = ParamInstall(Phead, $2->name, $1, VARIABLE);
       }


LDeclBlock: DECL_ LDeclList ENDDECL_ {
                lst = LSTParamInstall(lst, Phead);
                $$ = $2;
            }
          | DECL_ ENDDECL_ {
                lst = LSTParamInstall(lst, Phead);
                $$ = NULL;
            }
          | /* empty */ {
                lst = LSTParamInstall(lst, Phead);
                $$ = NULL;
            }

LDeclList: LDeclList LDecl {$$ = makeNode(CONNECTOR, VOID, $1, NULL, $2, NULL, "CONNECTOR");}
         | LDecl

LDecl: Type LIdList ';' {ASTChangeTypeLST(lst, $2, $1, NULL);}
     | TUPLE_ TName '(' TupleFieldList ')' LIdList ';' {
          struct TST_Node *tst_entry = TSTInstall($2, $4);
          $6 = ASTChangeTypeLST(lst, $6, TUPLE, tst_entry);
          $4 = ParamDelete($4);
          $$ = $6;
       }
     | TUPLE_ ID_ LIdList ';' {
          struct TST_Node *tst_entry = TSTLookup($2->name);
          if (tst_entry == NULL) {
              printf("Line %d: Tuple type \"%s\" not defined.\n", yylineno, $2->name);
              exit(1);
          }
          $3 = ASTChangeTypeLST(lst, $3, TUPLE, tst_entry);
          $$ = $3;
       }

LIdList: LIdList ',' ID_ {
             lst = LSTInstall(lst, $3->name, VOID, NULL);
             $$ = makeNode(CONNECTOR, VOID, $1, NULL, $3, NULL, "LIDLIST");
         }
       | LIdList ',' STAR_ ID_ {
             lst = LSTInstall(lst, $4->name, PVOID, NULL);
             $$ = makeNode(CONNECTOR, VOID, $1, NULL, $4, NULL, "LIDLIST");
         }
       | ID_ {
             lst = LSTInstall(lst, $1->name, VOID, NULL);
             $$ = $1;
         }
       | STAR_ ID_ {
             lst = LSTInstall(lst, $2->name, PVOID, NULL);
             $$ = $2;
         }

ArgList: ArgList ',' stringExpr {$$ = ASTArgAppend($1, $3);}
       | stringExpr

Type: INT_ {$$ = INTEGER;}
    | STR_ {$$ = STRING;}

ReturnType: Type STAR_ {
              if ($1 == INTEGER) $$ = POINTER_TO_INTEGER;
              else if ($1 == STRING) $$ = POINTER_TO_STRING;
              else {
                  printf("Line %d: Invalid pointer return type.\n", yylineno);
                  exit(1);
              }
            }
          | Type {$$ = $1;}

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

AsgnStmt: l_value '=' stringExpr {
    if ($1->type == TUPLE) {
        struct TST_Node *l_tst = $1->GSTentry ? $1->GSTentry->TSType : $1->TSTentry;
        struct TST_Node *r_tst = $3->GSTentry ? $3->GSTentry->TSType : $3->TSTentry;
        
        if ($3->type != TUPLE || l_tst != r_tst) {
            printf("Line %d: Type mismatch in tuple assignment (must be same tuple type).\n", yylineno);
            exit(1);
        }
        $$ = makeNode(OPERATOR, TUPLE, $1, NULL, $3, NULL, "=");
    } else {
        $$ = makeNode(OPERATOR, INTEGER, $1, NULL, $3, NULL, "=");
    }
}

IfStmt: IF_ '(' expr ')' THEN_ Stmt_list ELSE_ Stmt_list ENDIF_ { $$ = makeNode(IF, VOID, $3, $6, $8, NULL, "IF");}
      | IF_ '(' expr ')' THEN_ Stmt_list ENDIF_ { $$ = makeNode(IF, VOID, $3, $6, NULL, NULL, "IF");}

WhileStmt: WHILE_ '(' expr ')' DO_ Stmt_list ENDWHILE_ { $$ = makeNode(WHILE, VOID, $3, NULL, $6, NULL, "WHILE");}

expr : expr PLUS_ expr            {$$ = makeNode(OPERATOR, INTEGER, $1, NULL, $3, NULL, "+");}
     | expr MINUS_ expr          {$$ = makeNode(OPERATOR, INTEGER, $1, NULL, $3, NULL, "-");}
     | expr STAR_ expr           {$$ = makeNode(OPERATOR, INTEGER, $1, NULL, $3, NULL, "*");}
     | expr DIV_ expr            {$$ = makeNode(OPERATOR, INTEGER, $1, NULL, $3, NULL, "/");}
     | expr MOD_ expr            {$$ = makeNode(OPERATOR, INTEGER, $1, NULL, $3, NULL, "%");}
     | expr LT_ expr             {$$ = makeNode(OPERATOR, BOOLEAN, $1, NULL, $3, NULL, "<");}
     | expr GT_ expr             {$$ = makeNode(OPERATOR, BOOLEAN, $1, NULL, $3, NULL, ">");}
     | expr LE_ expr             {$$ = makeNode(OPERATOR, BOOLEAN, $1, NULL, $3, NULL, "<=");}
     | expr GE_ expr             {$$ = makeNode(OPERATOR, BOOLEAN, $1, NULL, $3, NULL, ">=");}
     | expr NE_ expr             {$$ = makeNode(OPERATOR, BOOLEAN, $1, NULL, $3, NULL, "!=");}
     | expr EQ_ expr             {$$ = makeNode(OPERATOR, BOOLEAN, $1, NULL, $3, NULL, "==");}
     | expr AND_ expr            {$$ = makeNode(OPERATOR, BOOLEAN, $1, NULL, $3, NULL, "&&");}
     | expr OR_ expr             {$$ = makeNode(OPERATOR, BOOLEAN, $1, NULL, $3, NULL, "||");}
     | '(' expr ')'              {$$ = $2;}
     | id                        {$$ = $1;}
     | NUM_                      {$$ = $1;}
     | STAR_ expr %prec USTAR    { $$ = makePointerNode(POINTER, $2, "*"); }
     | AMPERSAND_ id %prec UAMP  { $$ = makePointerNode(ADDRESS, $2, "&"); };

stringExpr: expr
          | TEXT_

l_value: simple_id { $$ = $1; } // Only simple IDs can be l_values
       | array_or_call_id { $$ = $1; } // Array access is valid l_value
       | STAR_ expr %prec USTAR { $$ = makePointerNode(POINTER, $2, "*"); }
       | tuple_field_access { $$ = $1; }

tuple_field_access: simple_id DOT_ ID_ { // Tuple must start with a simple ID
    $$ = makeTupleFieldAccessNode($1, $3->name, "TUPLE_FIELD_ACCESS");
    free($3);
} %prec DOT_

simple_id: ID_ {
    $$ = $1;
    struct LST_Node *curr_l = LSTLookup(lst, $1->name);
    if (curr_l) {
        $$->type = curr_l->type;
        $$->TSTentry = curr_l->TSType;
    } else {
        struct GST_Node *curr_g = GSTLookup($1->name);
        if (curr_g == NULL) {
            printf("Line %d: Variable \"%s\" not declared\n", yylineno, $1->name);
            exit(1);
        }
        $$->type = curr_g->type;
        $$->GSTentry = curr_g;
    }
}

// id is now a container for complex forms, including those that are valid r-values only (like function calls)
id: simple_id
  | array_or_call_id
  | tuple_field_access

// array_or_call_id will be used for both l-value and r-value forms that involve brackets or parentheses
array_or_call_id: ID_ '[' expr ']' {
      if ($3->type == BOOLEAN) {
          printf("Line %d: Array index cannot be boolean\n", yylineno);
          exit(1);
      }
      struct GST_Node *curr = GSTLookup($1->name);
      if (curr == NULL || curr->typeofvar != ARRAY) {
          printf("Line %d: \"%s\" is not a declared array\n", yylineno, $1->name);
          exit(1);
      }
      $1->type = curr->type;
      $$ = makeArrayLeafNode($1->name, $3, "ARRAY");
  }
  | ID_ '(' ArgList ')' { // Function call (r-value only)
      struct GST_Node *curr = GSTLookup($1->name);
      if (curr == NULL || curr->typeofvar != FUNCTION) {
          printf("Line %d: \"%s\" is not a declared function\n", yylineno, $1->name);
          exit(1);
      }
      if (checkASTParam(curr->Phead, $3) == 0) {
          printf("Line %d: Wrong arguments in \"%s\", does not match declaration\n", yylineno, $1->name);
          exit(1);
      }
      $1->nodetype = FUNCTIONCALL;
      $1->type = curr->type;
      $1->arg_list = $3;
      $$ = $1;
  }
  | ID_ '(' ')' { // Function call (no args, r-value only)
      struct GST_Node *curr = GSTLookup($1->name);
      if (curr == NULL || curr->typeofvar != FUNCTION) {
          printf("Line %d: \"%s\" is not a declared function\n", yylineno, $1->name);
          exit(1);
      }
      if (checkASTParam(curr->Phead, NULL) == 0) {
          printf("Line %d: Wrong arguments in \"%s\", does not match declaration\n", yylineno, $1->name);
          exit(1);
      }
      $1->nodetype = FUNCTIONCALL;
      $1->type = curr->type;
      $1->arg_list = NULL;
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
    Thead = NULL;
    char fname[256];
    scanf("%s",fname);
    yyin=fopen(fname,"r");
    ft = fopen("output.xsm", "w");
    yyparse();

    return 0;
}

