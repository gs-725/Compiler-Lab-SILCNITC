%{
    #include <stdlib.h>
    #include <stdio.h>
    #include "t.h"
    #include "t.c"

    extern FILE* yyin;
    extern char* yytext;
    extern int yylineno;

    void yyerror(char const *s);
    int yylex(void);

    struct LSTable *lst;
    struct ParamNode *Phead;
    struct FieldList *FL;
    struct ClassTable *Cptr, *currClass = NULL;

    FILE *ft;
%}

%union{
    struct AST_Node *ast;
    struct ParamNode *param;
    struct TypeTable *type;
}

%type <ast> Program
%type <ast> MainBlock Body
%type <ast> GDeclBlock GDeclList GDecl GIdList GId
%type <ast> FnDefBlock FnDef
%type <ast> LDeclBlock LDeclList LDecl LIdList
%type <ast> TypeDefBlock TypeDefList TypeDef FieldDeclList FieldDecl Field
%type <ast> ClassDefBlock ClassDefList ClassDef CFieldDeclList CFieldDecl Cname CMethodDef
%type <ast> FieldFunction
%type <ast> ArgLists ArgList
%type <ast> Stmt_list Stmt InputStmt OutputStmt AsgnStmt IfStmt WhileStmt
%type <ast> expr stringExpr
%type <ast> ID_ id
%type <param> ParamList Param
%type <type> Type FType
%token PLUS_ MINUS_ MUL_ DIV_ MOD_ LT_ GT_ LE_ GE_ NE_ EQ_ AND_ OR_ NOT_
%token BEGIN_ END_ READ_ WRITE_ IF_ THEN_ ELSE_ ENDIF_ WHILE_ DO_ ENDWHILE_
%token BREAK_ CONTINUE_ BREAKPOINT_
%token INITIALIZE_ ALLOC_ FREE_ SELF_
%token MAIN_ RETURN_
%token DECL_ ENDDECL_
%token INT_ STR_
%token ID_ NUM_ TEXT_
%token TYPE_ ENDTYPE_ NULL_
%token CLASS_ ENDCLASS_ EXTENDS_ NEW_ DELETE_
%left LT_ GT_ LE_ GE_ NE_ EQ_ AND_ OR_ NOT_
%left PLUS_ MINUS_
%left MUL_ DIV_ MOD_

%start Program

%%
//---------------------------------RULES---------------------------------------------

//---------------------------------PROGRAM--------------------------------------------
Program: TypeDefBlock ClassDefBlock GDeclBlock FnDefBlock MainBlock {
            $$ = NULL;
            printf("Parsing Successful\n");
            
            //printStack();

            generateHeader(ft);

            while(Shead != NULL) {
                struct Stack *top = popStack();
                codeGen(top->node, top->LST, top->class, ft);
            }

            fclose(ft);
            exit(0);
        }
        | TypeDefBlock ClassDefBlock GDeclBlock MainBlock {
            $$ = NULL;

            //printStack();

            printf("Parsing Successful\n");

            generateHeader(ft);

            while(Shead != NULL) {
                struct Stack *top = popStack();
                codeGen(top->node, top->LST, top->class, ft);
            }

            fclose(ft);
            exit(0);
        }


//--------------------------------MAIN BLOCK---------------------------------------------
MainBlock: INT_ MAIN_ '(' ')' '{' LDeclBlock Body '}' {
            if (!typeCheckInt($7->right)) {
                printf("Line %d: Return type of main function doesn't match\n", yylineno);
                exit(1);
            }
            GSTInstall("main", TTLookup("int"), 1, FUNCTION, NULL, NULL);
            struct GST_Node *main = GSTLookup("main");
            main->LST = LSTCopy(lst);
            lst = LSTDelete(lst);
            $$ = makeNode(FUNCTION, TTLookup("int"), NULL, $7, NULL, NULL, main, "main");
            $$->name = "main";
            //ASTPrint($$);
            pushStack($$, main->LST, NULL);
            //printf("\n");
        }


//---------------------------------BODY---------------------------------------------
Body: BEGIN_ Stmt_list RETURN_ stringExpr ';' END_ {
        struct AST_Node *temp = makeNode(RET, $4->type, NULL, $4, NULL, NULL, NULL, "RETURN");
        $$ = makeNode(CONNECTOR, TTLookup("void"), NULL, $2, NULL, temp, NULL, "BODY");
    }
    | BEGIN_ RETURN_ stringExpr ';' END_ {
        struct AST_Node *temp = makeNode(RET, $3->type, NULL, $3, NULL, NULL, NULL, "RETURN");
        $$ = makeNode(CONNECTOR, TTLookup("void"), NULL, NULL, NULL, temp, NULL, "BODY");
    }


//---------------------------------TYPE DEFINITIONS--------------------------------
TypeDefBlock  : TYPE_ TypeDefList ENDTYPE_ {/*TTPrint();*/}
              | {/*TTPrint();*/}

TypeDefList   : TypeDefList TypeDef
              | TypeDef

TypeDef       : ID_ '{' FieldDeclList '}' {
                    if (FL->size > 8) {
                        printf("Line %d: Size of user-defined type \'%s\' exceeds 8 bytes\n", yylineno, $1->name);
                        exit(1);
                    }
                    Thead = TTInstall($1->name, 8, FL);

                    struct TypeTable *type = TTLookup($1->name);
                    struct FieldListNode *f = FL->head;
                    while (f != NULL) {
                        if (f->type->size == 0) {
                            if (strcmp($1->name, f->type->name) == 0) {
                                f->type = type;
                            } else {
                                printf("Error: Invalid type %s\n", f->type->name);
                            }
                        }
                        f = f->next;
                    }
                    FL = FLInit();
                }

FieldDeclList : FieldDeclList FieldDecl
              | FieldDecl

FieldDecl   : FType ID_ ';' {FL = FLInstall(FL, $2->name, $1, NULL);}

FType   : INT_ {$$ = TTLookup("int");}
        | STR_ {$$ = TTLookup("str");}
        | ID_  {
            struct TypeTable *type = TTLookup($1->name);
            // Type not yet defined
            if (type == NULL) {
                type = (struct TypeTable *)malloc(sizeof(struct TypeTable));
                type->name = $1->name;
                type->size = 0;
            }
            $$ = type;
        }

//---------------------------------TYPE---------------------------------------------

Type: INT_ {$$ = TTLookup("int");}
    | STR_ {$$ = TTLookup("str");}
    | ID_  {
        if (TTLookup($1->name) == NULL) {
            printf("Line %d: Type \"%s\" not defined\n", yylineno, $1->name);
            exit(1);
        }
        $$ = TTLookup($1->name);
    }


//---------------------------------CLASS DECLARATIONS--------------------------------
ClassDefBlock   : CLASS_ ClassDefList ENDCLASS_ {/*CPrint(Chead);*/}
                |   {$$ = NULL;}

ClassDefList    : ClassDefList ClassDef
                | ClassDef

ClassDef        : Cname '{' DECL_ CFieldDeclList CMethodDecl ENDDECL_ CMethodDef '}' {
		     if (Cptr->methodCount > 8) {
                        printf("Line %d: Class \'%s\' can't have more than 8 membermethods\n", yylineno, Cptr->name);
                    				}                    
                    if (Cptr->fieldCount > 8) {
                        printf("Line %d: Class \'%s\' can't have more than 8 memberfields\n", yylineno, Cptr->name);
                    				}
                   
                    Cptr = NULL;
                }

Cname           : ID_ {
                    Cptr = CInstall($1->name, NULL);
                    TTInstall($1->name, 8, NULL);
                }
                | ID_ EXTENDS_ ID_ {
                    if (CLookup($3->name) == NULL) {
                        printf("Line %d: Class \'%s\' not defined\n", yylineno, $3->name);
                        exit(1);
                    }
                    Cptr = CInstall($1->name, $3->name);
                    Ccopyfields(Cptr);
                    Ccopymethods(Cptr);
                    TTInstall($1->name, 8, NULL);
                }

CFieldDeclList  : CFieldDeclList CFieldDecl
                | {$$ = NULL;}

CFieldDecl      : Type ID_ ';' {
                    if (Cptr == NULL) {
                        printf("Line %d: Class not defined\n", yylineno);
                        exit(1);
                    }
                    CInstallField(Cptr, $2->name, $1);
                }

CMethodDecl     : CMethodDecl CMDecl 
                | CMDecl

CMDecl          : Type ID_ '(' ParamList ')' ';' {
                    CInstallMethod(Cptr, $2->name, $1, ParamCopy(Phead));
                    Phead = ParamDelete(Phead);
                }
                | Type ID_ '(' ')' ';' {
                    CInstallMethod(Cptr, $2->name, $1, NULL);
                }

CMethodDef      : CMethodDef FnDef  {$$ = makeNode(CONNECTOR, TTLookup("void"), NULL, $1, NULL, $2, NULL, "CMETHODDEF");}
                | FnDef


//---------------------------------GLOBAL DECLARATIONS--------------------------------
GDeclBlock: DECL_ GDeclList ENDDECL_ {$$ = $2; /*GSTPrint();*/}
          | DECL_ ENDDECL_ {$$ = NULL;}

GDeclList: GDeclList GDecl
         | GDecl
        
GDecl: Type GIdList ';' {
            if (CLookup($1->name) != NULL) {
                ASTChangeTypeClassGST($2, NULL, CLookup($1->name), 2);
                SP += 1; // For class pointer
            } else {
                ASTChangeTypeClassGST($2, $1, NULL, -1);
            }
        }

GIdList: GIdList ',' GId {$$ = makeNode(CONNECTOR, TTLookup("void"), NULL, $1, NULL, $3, NULL, "GIDLIST");}
       | GId

GId: ID_ {
        GSTInstall($1->name, TTLookup("void"), 1, VARIABLE, NULL, NULL);
        $$ = $1;
    }
    | ID_ '[' NUM_ ']' {
        GSTInstall($1->name, TTLookup("void"), $<ast>3->val, ARRAY, NULL, NULL);
        $$ = $1;
    }
    | ID_ '(' ParamList ')' {
        GSTInstall($1->name, TTLookup("void"), 1, FUNCTION, ParamCopy(Phead), NULL);
        Phead = ParamDelete(Phead);
        $$ = $1;
    }
    | ID_ '(' ')' {
        GSTInstall($1->name, TTLookup("void"), 1, FUNCTION, NULL, NULL);
        $$ = $1;
    }


//--------------------------------FUNCTIONS---------------------------------------------
FnDefBlock: FnDefBlock FnDef
          | FnDef

FnDef: Type ID_ '(' ParamList ')' '{' LDeclBlock Body '}' {
            // Error if return type is of class type
            if (CLookup($1->name) != NULL) {
                printf("Line %d: Return type of function \"%s\" cannot be of class type\n", yylineno, $2->name);
                exit(1);
            }
            if(!inClass) {
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
                if ($7 == NULL) {
                    LSTParamInstall(lst, Phead);
                }
                f->LST = LSTCopy(lst);
                lst = LSTDelete(lst);
                $$ = makeNode(FUNCTION, $1, NULL, $8, NULL, ParamToArg(Phead), f, "FUNCTION");
                $$->name = $2->name;
                Phead = ParamDelete(Phead);
                //ASTPrint($$);
                pushStack($$, f->LST, NULL);
                //printf("\n");
            }
            else {
                struct MethodListNode *m = CLookupMethod(Cptr, $2->name);
                if (m == NULL || (!m->overridden && m->inherited)) {
                    printf("Line %d: Method \"%s\" not declared in class \"%s\"\n", yylineno, $2->name, Cptr->name);
                    exit(1);
                }
                if (m->type != $1) {
                    printf("Line %d: Invalid return type of method \"%s\"\n", yylineno, $2->name);
                    exit(1);
                }
                if (ParamCheck(m->Phead, Phead) == 0) {
                    printf("Line %d: Parameter list of method \"%s\" doesn't match\n", yylineno, $2->name);
                    exit(1);
                }
                if ($8->right->type != $1) {
                    printf("Line %d: Return type of method \"%s\" doesn't match\n", yylineno, $2->name);
                    exit(1);
                }
                m->LST = LSTCopy(lst);
                lst = LSTDelete(lst);
                $$ = makeNode(FUNCTION, $1, NULL, $8, NULL, ParamToArg(Phead), NULL, "METHOD");
                $$->name = $2->name;
                $$->class = Cptr;
                Phead = ParamDelete(Phead);
                //ASTPrint($$);
                pushStack($$, m->LST, Cptr);
                //printf("\n");
            }
        }
        | Type ID_ '(' ')' '{' LDeclBlock Body '}' {
            // Error if return type is of class type
            if (CLookup($1->name) != NULL) {
                printf("Line %d: Return type of function \"%s\" cannot be of class type\n", yylineno, $2->name);
                exit(1);
            }
            if (!inClass) {
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
                $$ = makeNode(FUNCTION, $1, NULL, $7, NULL, NULL, f, "FUNCTION");
                $$->name = $2->name;
                //ASTPrint($$);
                pushStack($$, f->LST, NULL);
                //printf("\n");
            }
            else {
                struct MethodListNode *m = CLookupMethod(Cptr, $2->name);
                if (m == NULL) {
                    printf("Line %d: Method \"%s\" not declared in class \"%s\"\n", yylineno, $2->name, Cptr->name);
                    exit(1);
                }
                if (m->type != $1) {
                    printf("Line %d: Invalid return type of method \"%s\"\n", yylineno, $2->name);
                    exit(1);
                }
                if (ParamCheck(m->Phead, NULL) == 0) {
                    printf("Line %d: Parameter list of method \"%s\" doesn't match\n", yylineno, $2->name);
                    exit(1);
                }
                if ($7->right->type != $1) {
                    printf("Line %d: Return type of method \"%s\" doesn't match\n", yylineno, $2->name);
                    exit(1);
                }
                m->LST = LSTCopy(lst);
                lst = LSTDelete(lst);
                $$ = makeNode(FUNCTION, $1, NULL, $7, NULL, NULL, NULL, "METHOD");
                $$->name = $2->name;
                $$->class = Cptr;
                //ASTPrint($$);
                pushStack($$, m->LST, Cptr);
               // printf("\n");
            }
        }


//--------------------------------PARAMETERS---------------------------------------------
ParamList: ParamList ',' Param
         | Param

Param:  Type ID_ {
            if (CLookup($1->name) != NULL) {
                printf("Line %d: Invalid parameter type \"%s\" (Classes cannot be parameters)\n", yylineno, $1->name);
                exit(1);
            }
            Phead = ParamInstall(Phead, $2->name, $1);
        }


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
            | {
                lst = LSTParamInstall(lst, Phead);
                //LSTPrint(lst);
                $$ = NULL;
            }

LDeclList: LDeclList LDecl
         | LDecl

LDecl:  Type LIdList ';' {
            if (CLookup($1->name) != NULL) {
                printf("Line %d: Invalid local declaration type \"%s\" (Classes cannot be local declarations)\n", yylineno, $1->name);
                exit(1);
            }
            ASTChangeTypeLST(lst, $2, $1);
        }

LIdList: LIdList ',' ID_ {
            lst = LSTInstall(lst, $3->name, TTLookup("void"), NULL);
            $$ = makeNode(CONNECTOR, TTLookup("void"), NULL, $1, NULL, $3, NULL, "LIDLIST");
        }
        | ID_ {
            lst = LSTInstall(lst, $1->name, TTLookup("void"), NULL);
            $$ = $1;
        }


//--------------------------------ARGUMENTS---------------------------------------------
ArgList: ArgLists   {$$ = $1;}
       |            {$$ = NULL;}

ArgLists: ArgLists ',' stringExpr {$$ = ASTArgAppend($1, $3);}
        | stringExpr



//---------------------------------STATEMENTS---------------------------------------------

Stmt_list: Stmt_list Stmt ';' {$$ = makeNode(CONNECTOR, TTLookup("void"), NULL, $1, NULL, $2, NULL, "CONNECTOR");}
    | Stmt ';'

Stmt: InputStmt
    | OutputStmt
    | AsgnStmt
    | IfStmt
    | WhileStmt
    | BREAK_                { $$ = makeNode(BREAK, TTLookup("void"), NULL, NULL, NULL, NULL, NULL, "BREAK"); }
    | CONTINUE_             { $$ = makeNode(CONTINUE, TTLookup("void"), NULL, NULL, NULL, NULL, NULL, "CONTINUE"); }
    | BREAKPOINT_           { $$ = makeNode(BREAKPOINT, TTLookup("void"), NULL, NULL, NULL, NULL, NULL, "BREAKPOINT"); }
    | INITIALIZE_ '(' ')'   { $$ = makeNode(INITIALIZE, TTLookup("void"), NULL, NULL, NULL, NULL, NULL, "INITIALIZE"); }

InputStmt:  READ_ '(' id ')' {
                if ($3->nodetype == FUNCTION) {
                    printf("Line %d: Cannot read into function \"%s\"\n", yylineno, $3->name);
                    exit(1);
                }
                if ($3->type == TTLookup("int") || $3->type == TTLookup("str"))
                    $$ = makeNode(READ, TTLookup("void"), NULL, $3, NULL, NULL, NULL, "READ");
                else {
                    printf("Line %d: Cannot read into non-primitive data type\n", yylineno);
                    exit(1);
                }
            }

OutputStmt: WRITE_ '(' stringExpr ')' { $$ = makeNode(WRITE, TTLookup("void"), NULL, $3, NULL, NULL, NULL, "WRITE");}

AsgnStmt: id '=' stringExpr {
            if($1->type == $3->type && ($1->nodetype != FUNCTION))
            {
                $$ = makeNode(OPERATOR, TTLookup("void"), NULL, $1, NULL, $3, NULL, "=");
            }
            else if ($1->class && $3->class && CInheritanceCheck($1->class, $3->class))
            {
                $$ = makeNode(OPERATOR, TTLookup("void"), NULL, $1, NULL, $3, NULL, "=");
            }
            else {
                printf("Line %d: Invalid assignment\n", yylineno);
                exit(1);
            }
        }
        | id '=' NULL_ {
            if($1->type != TTLookup("int") && $1->type != TTLookup("str") && $1->nodetype != FUNCTION)
            {
                $$ = makeNode(OPERATOR, TTLookup("void"), NULL, $1, NULL, $<ast>3, NULL, "=");
            }
            else
            {
                printf("Line %d: NULL cannot be assigned to primitive data-types\n", yylineno);
                exit(1);
            }
        }
        | id '=' ALLOC_ '(' ')' {
            if($1->type != TTLookup("int") && $1->type != TTLookup("str") && $1->nodetype != FUNCTION)
            {
                $$ = makeNode(OPERATOR, TTLookup("void"), NULL, $1, NULL, $<ast>3, NULL, "=");
            }
            else
            {
                printf("Line %d: alloc() cannot be assigned to primitive data-types\n", yylineno);
                exit(1);
            }
        }
        | id '=' FREE_ '(' id ')' {
            if ($1->type == TTLookup("int") && $1->nodetype != FUNCTION && $5->type != TTLookup("int") && $5->type != TTLookup("str") && $5->nodetype != ARRAY && $5->nodetype != FUNCTION)
            {
                $<ast>3->left = $5;
                $$ = makeNode(OPERATOR, TTLookup("void"), NULL, $1, NULL, $<ast>3, NULL, "=");
            }
            else
            {
                printf("Line %d: Semantic error in free()\n", yylineno);
                exit(1);
            }
        }
        | id '=' NEW_ '(' ID_ ')' {
            struct ClassTable *newClass = CLookup($5->name);
            if (newClass == NULL) {
                printf("Line %d: Class \"%s\" not defined\n", yylineno, $5->name);
                exit(1);
            }
            if (($1->type == NULL) && $1->class && $1->nodetype != FUNCTION) {
                if (CInheritanceCheck($1->class, newClass)) {
                    struct AST_Node *l = makeNode(OPERATOR, TTLookup("void"), NULL, $1, NULL, $<ast>3, NULL, "=");
                    $$ = makeNode(NEW, TTLookup("void"), NULL, l, NULL, $5, NULL, "NEW");
                }
                else {
                    printf("Line %d: Class \"%s\" is not a subclass of \"%s\"\n", yylineno, $1->class->name, $5->name);
                    exit(1);
                }
            }
            else {
                printf("Line %d: Cannot assign to non-object\n", yylineno);
                exit(1);
            }
        }
        | DELETE_ '(' id ')' {
            if ($3->type == NULL && $3->class && $3->nodetype != FUNCTION && $3->nodetype != ARRAY) {
                $$ = makeNode(DELETE, TTLookup("void"), NULL, $<ast>1, NULL, $3, NULL, "DELETE");
            }
            else {
                printf("Line %d: Cannot delete non-object\n", yylineno);
                exit(1);
            }
        }

IfStmt: IF_ '(' expr ')' THEN_ Stmt_list ELSE_ Stmt_list ENDIF_ { $$ = makeNode(IF, TTLookup("void"), NULL, $3, $6, $8, NULL, "IF");}
    | IF_ '(' expr ')' THEN_ Stmt_list ENDIF_ { $$ = makeNode(IF, TTLookup("void"), NULL, $3, $6, NULL, NULL, "IF");}

WhileStmt: WHILE_ '(' expr ')' DO_ Stmt_list ENDWHILE_ { $$ = makeNode(WHILE, TTLookup("void"), NULL, $3, NULL, $6, NULL, "WHILE");}

//---------------------------------EXPRESSIONS---------------------------------------------

expr : expr PLUS_ expr		{$$ = makeNode(OPERATOR, TTLookup("int"), NULL, $1, NULL, $3, NULL, "+");}
    | expr MINUS_ expr		{$$ = makeNode(OPERATOR, TTLookup("int"), NULL, $1, NULL, $3, NULL, "-");}
    | expr MUL_ expr		{$$ = makeNode(OPERATOR, TTLookup("int"), NULL, $1, NULL, $3, NULL, "*");}
    | expr DIV_ expr		{$$ = makeNode(OPERATOR, TTLookup("int"), NULL, $1, NULL, $3, NULL, "/");}
    | expr MOD_ expr		{$$ = makeNode(OPERATOR, TTLookup("int"), NULL, $1, NULL, $3, NULL, "%");}
    | expr LT_ expr			{$$ = makeNode(OPERATOR, TTLookup("boolean"), NULL, $1, NULL, $3, NULL, "<");}
    | expr GT_ expr			{$$ = makeNode(OPERATOR, TTLookup("boolean"), NULL, $1, NULL, $3, NULL, ">");}
    | expr LE_ expr			{$$ = makeNode(OPERATOR, TTLookup("boolean"), NULL, $1, NULL, $3, NULL, "<=");}
    | expr GE_ expr			{$$ = makeNode(OPERATOR, TTLookup("boolean"), NULL, $1, NULL, $3, NULL, ">=");}
    | expr NE_ expr			{$$ = makeNode(OPERATOR, TTLookup("boolean"), NULL, $1, NULL, $3, NULL, "!=");}
    | expr EQ_ expr			{$$ = makeNode(OPERATOR, TTLookup("boolean"), NULL, $1, NULL, $3, NULL, "==");}
    | expr NE_ NULL_        {$$ = makeNode(OPERATOR, TTLookup("boolean"), NULL, $1, NULL, $<ast>3, NULL, "!=");}
    | expr EQ_ NULL_        {$$ = makeNode(OPERATOR, TTLookup("boolean"), NULL, $1, NULL, $<ast>3, NULL, "==");}
    | expr AND_ expr		{$$ = makeNode(OPERATOR, TTLookup("boolean"), NULL, $1, NULL, $3, NULL, "AND");}
    | expr OR_ expr			{$$ = makeNode(OPERATOR, TTLookup("boolean"), NULL, $1, NULL, $3, NULL, "OR");}
    | NOT_ expr				{$$ = makeNode(OPERATOR, TTLookup("boolean"), NULL, $2, NULL, NULL, NULL, "NOT");}
    | '(' expr ')' 			{$$ = $2;}
    | id
    | NUM_                  {$$ = $<ast>1;}

stringExpr: expr
          | TEXT_   {$$ = $<ast>1;}

//---------------------------------IDENTIFIERS---------------------------------------------
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
            $$->class = curr->class;
        }
    }
    | ID_ '[' expr ']' {
        // Error if expr is boolean
        if (!typeCheckInt($3)) {
            printf("Line %d: Invalid index\n", yylineno);
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
    | Field
    | FieldFunction

//---------------------------------FIELD---------------------------------------------
Field:  Field '.' ID_ {
            if (currClass != NULL) {
                printf("Line %d: Private member access error\n", yylineno);
                exit(1);
            }
            struct FieldListNode *f = TTLookupField($1->type, $3->name);
            if (f == NULL) {
                printf("Line %d: Field \"%s\" not found in type \"%s\"\n", yylineno, $3->name, $1->type->name);
                exit(1);
            }
            $3->type = f->type;
            $$ = makeNode(FIELD, f->type, NULL, $1, NULL, $3, NULL, "FIELD");
            currClass = NULL;
        }
        | ID_ '.' ID_ {
            if (CLookup($1->name) != NULL) {
                printf("Line %d: Data members can only be accessed through member functions\n", yylineno);
                exit(1);
            }
            struct TypeTable *TT = NULL;
            struct LST_Node *curr = LSTLookup(lst, $1->name);
            if (curr) {
                TT = curr->type;
            } else {
                struct GST_Node *curr = GSTLookup($1->name);
                if (curr == NULL) {
                    printf("Line %d: Variable \"%s\" not declared\n", yylineno, $1->name);
                    exit(1);
                }
                TT = curr->type;
            }
            if (TT == NULL) {
                printf("Line %d: Private member access error\n", yylineno);
                exit(1);
            }
            struct FieldListNode *f = TTLookupField(TT, $3->name);
            if (f == NULL) {
                printf("Line %d: Field \"%s\" not found in \"%s\"\n", yylineno, $3->name, TT->name);
                exit(1);
            }
            $1->type = TT;
            $3->type = f->type;
            $$ = makeNode(FIELD, f->type, NULL, $1, NULL, $3, NULL, "FIELD");
        }
        | SELF_ '.' ID_ {
            currClass = NULL;
            if (!inClass) {
                printf("Line %d: \"self\" can only be used inside a class\n", yylineno);
                exit(1);
            }
            $<ast>1->type = NULL;
            $<ast>1->class = Cptr;
            struct FieldListNode *f = CLookupField(Cptr, $3->name);
            if (f == NULL) {
                printf("Line %d: Field \"%s\" not found in class \"%s\"\n", yylineno, $3->name, Cptr->name);
                exit(1);
            }
            $3->type = f->type;
            $3->class = f->class;
            $$ = makeNode(FIELD, f->type, f->class, $<ast>1, NULL, $3, NULL, "FIELD");
            currClass = f->class;
        }

FieldFunction:  Field '.' ID_ '(' ArgList ')' {
                    struct MethodListNode *m = CLookupMethod(currClass, $3->name);
                    if (m == NULL) {
                        printf("currClass = %s\n", currClass->name);
                        printf("Line %d: Method \"%s\" not found in class \"%s\"\n", yylineno, $3->name, currClass->name);
                        exit(1);
                    }
                    if (checkASTParam(m->Phead, $5) == 0) {
                        printf("Line %d: Wrong arguments in \"%s\", does not match with declaration\n", yylineno, $3->name);
                        exit(1);
                    }
                    $3->nodetype = FUNCTIONCALL;
                    $3->type = m->type;
                    $3->class = NULL;
                    $3->arg_list = $5;
                    $$ = makeNode(FIELDFUNCTIONCALL, m->type, NULL, $1, NULL, $3, NULL, $3->name);
                    currClass = NULL;
                }
                | ID_ '.' ID_ '(' ArgList ')' {
                    struct GST_Node *g = GSTLookup($1->name);
                    if (g == NULL) {
                        printf("Line %d: Variable \"%s\" not declared\n", yylineno, $1->name);
                        exit(1);
                    }
                    if (g->class == NULL) {
                        printf("Line %d: \"%s\" is not a class variable\n", yylineno, $1->name);
                        exit(1);
                    }
                    struct MethodListNode *m = CLookupMethod(g->class, $3->name);
                    if (m == NULL) {
                        printf("Line %d: Method \"%s\" not found in class \"%s\"\n", yylineno, $3->name, g->class->name);
                        exit(1);
                    }
                    if (checkASTParam(m->Phead, $5) == 0) {
                        printf("Line %d: Wrong arguments in \"%s\", does not match with declaration\n", yylineno, $3->name);
                        exit(1);
                    }
                    $1->nodetype = VARIABLE;
                    $1->type = g->type;
                    $1->class = g->class;
                    $3->nodetype = FUNCTIONCALL;
                    $3->type = m->type;
                    $3->class = NULL;
                    $3->arg_list = $5;
                    $$ = makeNode(FIELDFUNCTIONCALL, m->type, NULL, $1, NULL, $3, NULL, $3->name);
                }
                | SELF_ '.' ID_ '(' ArgList ')' {
                    if (!inClass) {
                        printf("Line %d: \"self\" can only be used inside a class\n", yylineno);
                        exit(1);
                    }
                    struct MethodListNode *m = CLookupMethod(Cptr, $3->name);
                    if (m == NULL) {
                        printf("Line %d: Method \"%s\" not found in class \"%s\"\n", yylineno, $3->name, Cptr->name);
                        exit(1);
                    }
                    if (checkASTParam(m->Phead, $5) == 0) {
                        printf("Line %d: Wrong arguments in \"%s\", does not match with declaration\n", yylineno, $3->name);
                        exit(1);
                    }
                    $<ast>1->nodetype = VARIABLE;
                    $<ast>1->type = NULL;
                    $<ast>1->class = Cptr;
                    $3->nodetype = FUNCTIONCALL;
                    $3->type = m->type;
                    $3->class = NULL;
                    $3->arg_list = $5;
                    $$ = makeNode(FIELDFUNCTIONCALL, m->type, NULL, $<ast>1, NULL, $3, NULL, $3->name);
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
    FL = FLInit();
    TTCreate();
    char fname[256];
    scanf("%s",fname);
    yyin=fopen(fname,"r");
    ft = fopen("output.xsm", "w");
    yyparse();

    return 0;
}
