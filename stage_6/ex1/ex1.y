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
    struct Fieldlist *FL;

    FILE *ft;
%}

%union{
    struct AST_Node *ast;
    struct ParamNode *param;
    struct Typetable *type;
}

%type <ast> Program
%type <ast> MainBlock Body
%type <ast> GDeclBlock GDeclList GDecl GIdList GId
%type <ast> FnDefBlock FnDef
%type <ast> LDeclBlock LDeclList LDecl LIdList
%type <ast> TypeDefBlock TypeDefList TypeDef FieldDeclList FieldDecl Field
%type <ast> ArgList
%type <ast> Stmt_list Stmt InputStmt OutputStmt AsgnStmt IfStmt WhileStmt
%type <ast> expr stringExpr
%type <ast> ALLOC_ FREE_
%type <ast> ID_ NUM_ TEXT_ id NULL_
%type <param> ParamList Param
%type <type> Type
%token PLUS_ MINUS_ MUL_ DIV_ MOD_ LT_ GT_ LE_ GE_ NE_ EQ_ AND_ OR_ NOT_
%token BEGIN_ END_ READ_ WRITE_ IF_ THEN_ ELSE_ ENDIF_ WHILE_ DO_ ENDWHILE_
%token BREAK_ CONTINUE_ BREAKPOINT_
%token INITIALIZE_ ALLOC_ FREE_
%token MAIN_ RETURN_
%token DECL_ ENDDECL_
%token INT_ STR_
%token ID_ NUM_ TEXT_
%token TYPE_ ENDTYPE_ NULL_
%left LT_ GT_ LE_ GE_ NE_ EQ_ AND_ OR_ NOT_
%left PLUS_ MINUS_
%left MUL_ DIV_ MOD_

%start Program

%%
//---------------------------------RULES---------------------------------------------

//---------------------------------PROGRAM--------------------------------------------
Program: TypeDefBlock GDeclBlock FnDefBlock MainBlock {
            $$ = NULL;
            printf("Parsing Successful\n");
            
            generateHeader(ft);
            
            // Generate code for functions
            while (strcmp($3->s, "FNDEFBLOCK") == 0) {
                codeGen($3->right, $3->right->GSTentry->LST, ft);
                $3 = $3->left;
            }
            codeGen($3, $3->GSTentry->LST, ft);

            // Generate code for main
            codeGen($4, $4->GSTentry->LST, ft);

            fclose(ft);
            exit(0);
        }
        | TypeDefBlock GDeclBlock MainBlock {
            $$ = NULL;
            printf("Parsing Successful\n");

            generateHeader(ft);

            // Generate code for functions
            codeGen($3, $3->GSTentry->LST, ft);

            fclose(ft);
            exit(0);
        }
        | TypeDefBlock MainBlock {
            $$ = NULL;
            printf("Parsing Successful\n");

            generateHeader(ft);

            codeGen($2, $2->GSTentry->LST, ft);

            fclose(ft);
            exit(0);
        }


//--------------------------------MAIN BLOCK---------------------------------------------
MainBlock: INT_ MAIN_ '(' ')' '{' LDeclBlock Body '}' {
            if (!typeCheckInt($7->right)) {
                printf("Line %d: Return type of main function doesn't match\n", yylineno);
                exit(1);
            }
            GSTInstall("main", TTLookup("int"), 1, FUNCTION, NULL);
            struct GST_Node *main = GSTLookup("main");
            // NOTE: For local array support, the LST size needs updating before this.
            // LST size is updated in the LDeclBlock rule, so this order is fine.
            main->LST = LSTCopy(lst);
            lst = LSTDelete(lst);
            $$ = makeNode(FUNCTION, TTLookup("int"), $7, NULL, NULL, main, "main");
            //ASTPrint($$);
            //printf("\n");
        }


//---------------------------------BODY---------------------------------------------
Body: BEGIN_ Stmt_list RETURN_ stringExpr ';' END_ {
        struct AST_Node *temp = makeNode(RET, $4->type, $4, NULL, NULL, NULL, "RETURN");
        $$ = makeNode(CONNECTOR, TTLookup("void"), $2, NULL, temp, NULL, "BODY");
    }
    | BEGIN_ RETURN_ stringExpr ';' END_ {
        struct AST_Node *temp = makeNode(RET, $3->type, $3, NULL, NULL, NULL, "RETURN");
        $$ = makeNode(CONNECTOR, TTLookup("void"), NULL, NULL, temp, NULL, "BODY");
    }


//---------------------------------TYPE DEFINITIONS--------------------------------
TypeDefBlock  : TYPE_ TypeDefList ENDTYPE_ {TTPrint();}
              | {TTPrint();}

TypeDefList   : TypeDefList TypeDef
              | TypeDef

TypeDef       : ID_ '{' FieldDeclList '}' {
                    if (FL->size > 8) {
                        printf("Line %d: Size of user-defined type \'%s\' exceeds 8 bytes\n", yylineno, $1->name);
                        exit(1);
                    }
                    Thead = TTInstall($1->name, 8, FL);

                    struct Typetable *type = TTLookup($1->name);
                    struct FieldlistNode *f = FL->head;
                    while (f != NULL) {
                        if (f->type->size == 0) {
                            if (strcmp($1->name, f->type->name) == 0) {
                                f->type = type;
                            } else {
                                printf("Error: Invalid type \"%s\"\n", f->type->name);
                                exit(1);
                            }
                        }
                        f = f->next;
                    }
                    FL = FLInit();
                }

FieldDeclList : FieldDeclList FieldDecl
              | FieldDecl

FieldDecl   : INT_ ID_ ';' {
                struct Typetable *type = TTLookup("int");
                FL = FLInstall(FL, $2->name, type);
            }
            | STR_ ID_ ';' {
                struct Typetable *type = TTLookup("str");
                FL = FLInstall(FL, $2->name, type);
            }
            | ID_ ID_ ';' {
                struct Typetable *type = TTLookup($1->name);
                // Type not yet defined
                if (type == NULL) {
                    type = (struct Typetable *)malloc(sizeof(struct Typetable));
                    type->name = $1->name;
                    type->size = 0;
                }
                FL = FLInstall(FL, $2->name, type);
            }

Type: INT_ {$$ = TTLookup("int");}
    | STR_ {$$ = TTLookup("str");}
    | ID_  {
        if (TTLookup($1->name) == NULL) {
            printf("Line %d: Type %s not defined\n", yylineno, $1->name);
            exit(1);
        }
        $$ = TTLookup($1->name);
    }



//---------------------------------GLOBAL DECLARATIONS--------------------------------
GDeclBlock: DECL_ GDeclList ENDDECL_ {$$ = $2; GSTPrint();}
          | DECL_ ENDDECL_ {$$ = NULL;}

GDeclList: GDeclList GDecl
         | GDecl
        
GDecl: Type GIdList ';' {ASTChangeTypeGST($2, $1);}

GIdList: GIdList ',' GId {$$ = makeNode(CONNECTOR, TTLookup("void"), $1, NULL, $3, NULL, "GIDLIST");}
       | GId

GId: ID_ {
        GSTInstall($1->name, TTLookup("void"), 1, VARIABLE, NULL);
        $$ = $1;
    }
    | ID_ '[' NUM_ ']' {
        // Allows arrays of primitive types and UDT arrays (arrays of pointers)
        GSTInstall($1->name, TTLookup("void"), $3->val, ARRAY, NULL);
        $$ = $1;
    }
    | ID_ '(' ParamList ')' {
        GSTInstall($1->name, TTLookup("void"), 1, FUNCTION, ParamCopy(Phead));
        Phead = ParamDelete(Phead);
        $$ = $1;
    }
    | ID_ '(' ')' {
        GSTInstall($1->name, TTLookup("void"), 1, FUNCTION, NULL);
        $$ = $1;
    }


//--------------------------------FUNCTIONS---------------------------------------------
FnDefBlock: FnDefBlock FnDef {$$ = makeNode(CONNECTOR, TTLookup("void"), $1, NULL, $2, NULL, "FNDEFBLOCK");}
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
            printf("\n");
        }


//--------------------------------PARAMETERS---------------------------------------------
ParamList: ParamList ',' Param
         | Param

Param: Type ID_ {Phead = ParamInstall(Phead, $2->name, $1, VARIABLE);}


//--------------------------------LOCAL DECLARATIONS-------------------------------------
LDeclBlock: DECL_ LDeclList ENDDECL_ {
                lst = LSTParamInstall(lst, Phead);
                LSTPrint(lst);
                $$ = $2;
            }
            | DECL_ ENDDECL_ {
                lst = LSTParamInstall(lst, Phead);
                LSTPrint(lst);
                $$ = NULL;
            }
            | {
                lst = LSTParamInstall(lst, Phead);
                LSTPrint(lst);
                $$ = NULL;
            }

LDeclList: LDeclList LDecl
         | LDecl

LDecl: Type LIdList ';' {
        struct LST_Node *curr = lst->head;
        int current_binding = 0;
        int max_binding = 0;
        
        while(curr) {
            // Negative binding means it's a parameter (already handled by LSTParamInstall)
            if (curr->binding > 0) {
                // For a variable or array: curr->binding currently stores its size (1 or N)
                current_binding += curr->binding; 
                
                // Set the correct final binding (offset from BP)
                // The new variable/array STARTS at current_binding - (size) + 1.
                // Example: length(1) is at 1. x(1) is at 2. p[2] is at 3.
                curr->binding = current_binding - curr->binding + 1; 
            } 
            
            // max_binding tracks the total size of the local variable section.
            if(current_binding > max_binding) max_binding = current_binding;
            curr = curr->next;
        }
        
        // This calculates the correct LST offsets and sets the types.
        ASTChangeTypeLST(lst, $2, $1);
        
        // Update size of LST table to be total stack space required for locals.
        lst->size = max_binding;
    }

LIdList: LIdList ',' ID_ {
            lst = LSTInstall(lst, $3->name, TTLookup("void"));
            $$ = makeNode(CONNECTOR, TTLookup("void"), $1, NULL, $3, NULL, "LIDLIST");
        }
        | ID_ {
            lst = LSTInstall(lst, $1->name, TTLookup("void"));
            $$ = $1;
        }
        // NEW: Local array declaration
        | LIdList ',' ID_ '[' NUM_ ']' { 
            // The logic here is simplified: we store array size in the binding field temporarily
            // The correct binding offset (from BP) will be calculated in the LDecl rule.
            lst = LSTInstall(lst, $3->name, TTLookup("void")); 
            LSTLookup(lst, $3->name)->binding = $5->val; // Store array size temporarily
            $$ = makeNode(CONNECTOR, TTLookup("void"), $1, NULL, $3, NULL, "LIDLIST");
        }
        | ID_ '[' NUM_ ']' {
            // New base case for local array declaration
            lst = LSTInstall(lst, $1->name, TTLookup("void"));
            LSTLookup(lst, $1->name)->binding = $3->val; // Store array size temporarily
            $$ = $1;
        }


//--------------------------------ARGUMENTS---------------------------------------------
ArgList: ArgList ',' stringExpr {$$ = ASTArgAppend($1, $3);}
       | stringExpr



//---------------------------------STATEMENTS---------------------------------------------

Stmt_list: Stmt_list Stmt ';' {$$ = makeNode(CONNECTOR, TTLookup("void"), $1, NULL, $2, NULL, "CONNECTOR");}
    | Stmt ';'

Stmt: InputStmt
    | OutputStmt
    | AsgnStmt
    | IfStmt
    | WhileStmt
    | BREAK_                { $$ = makeNode(BREAK, TTLookup("void"), NULL, NULL, NULL, NULL, "BREAK"); }
    | CONTINUE_             { $$ = makeNode(CONTINUE, TTLookup("void"), NULL, NULL, NULL, NULL, "CONTINUE"); }
    | BREAKPOINT_           { $$ = makeNode(BREAKPOINT, TTLookup("void"), NULL, NULL, NULL, NULL, "BREAKPOINT"); }
    | INITIALIZE_ '(' ')'   { $$ = makeNode(INITIALIZE, TTLookup("void"), NULL, NULL, NULL, NULL, "INITIALIZE"); }

InputStmt: READ_ '(' id ')' { $$ = makeNode(READ, TTLookup("void"), $3, NULL, NULL, NULL, "READ");}

OutputStmt: WRITE_ '(' stringExpr ')' { $$ = makeNode(WRITE, TTLookup("void"), $3, NULL, NULL, NULL, "WRITE");}

AsgnStmt: id '=' stringExpr {
            if($1->type == $3->type && ($1->nodetype != FUNCTION))
            {
                $$ = makeNode(OPERATOR, TTLookup("void"), $1, NULL, $3, NULL, "=");
            } else {
                printf("Line %d: Invalid assignment\n", yylineno);
                exit(1);
            }
        }
        | id '=' NULL_ {
            if($1->type != TTLookup("int") && $1->type != TTLookup("str") && $1->nodetype != FUNCTION)
            {
                $$ = makeNode(OPERATOR, TTLookup("void"), $1, NULL, $3, NULL, "=");
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
                $$ = makeNode(OPERATOR, TTLookup("void"), $1, NULL, $3, NULL, "=");
            }
            else
            {
                printf("Line %d: alloc() cannot be assigned to primitive data-types\n", yylineno);
                exit(1);
            }
        }
        | id '=' FREE_ '(' id ')' {
            // Left side must be an integer (result of free)
            // Right side operand must be a UDT variable/field/array-element
            if ($1->type == TTLookup("int") && $1->nodetype != FUNCTION && $5->type != TTLookup("int") && $5->type != TTLookup("str") && $5->nodetype != FUNCTION)
            {
                $3->left = $5;
                $$ = makeNode(OPERATOR, TTLookup("void"), $1, NULL, $3, NULL, "=");
            }
            else
            {
                printf("Line %d: Semantic error in free()\n", yylineno);
                exit(1);
            }
        }

IfStmt: IF_ '(' expr ')' THEN_ Stmt_list ELSE_ Stmt_list ENDIF_ { $$ = makeNode(IF, TTLookup("void"), $3, $6, $8, NULL, "IF");}
    | IF_ '(' expr ')' THEN_ Stmt_list ENDIF_ { $$ = makeNode(IF, TTLookup("void"), $3, $6, NULL, NULL, "IF");}

WhileStmt: WHILE_ '(' expr ')' DO_ Stmt_list ENDWHILE_ { $$ = makeNode(WHILE, TTLookup("void"), $3, NULL, $6, NULL, "WHILE");}

//---------------------------------EXPRESSIONS---------------------------------------------

expr : expr PLUS_ expr		{$$ = makeNode(OPERATOR, TTLookup("int"), $1, NULL, $3, NULL, "+");}
    | expr MINUS_ expr		{$$ = makeNode(OPERATOR, TTLookup("int"), $1, NULL, $3, NULL, "-");}
    | expr MUL_ expr		{$$ = makeNode(OPERATOR, TTLookup("int"), $1, NULL, $3, NULL, "*");}
    | expr DIV_ expr		{$$ = makeNode(OPERATOR, TTLookup("int"), $1, NULL, $3, NULL, "/");}
    | expr MOD_ expr		{$$ = makeNode(OPERATOR, TTLookup("int"), $1, NULL, $3, NULL, "%");}
    | expr LT_ expr			{$$ = makeNode(OPERATOR, TTLookup("boolean"), $1, NULL, $3, NULL, "<");}
    | expr GT_ expr			{$$ = makeNode(OPERATOR, TTLookup("boolean"), $1, NULL, $3, NULL, ">");}
    | expr LE_ expr			{$$ = makeNode(OPERATOR, TTLookup("boolean"), $1, NULL, $3, NULL, "<=");}
    | expr GE_ expr			{$$ = makeNode(OPERATOR, TTLookup("boolean"), $1, NULL, $3, NULL, ">=");}
    | expr NE_ expr			{$$ = makeNode(OPERATOR, TTLookup("boolean"), $1, NULL, $3, NULL, "!=");}
    | expr EQ_ expr			{$$ = makeNode(OPERATOR, TTLookup("boolean"), $1, NULL, $3, NULL, "==");}
    | expr NE_ NULL_        {$$ = makeNode(OPERATOR, TTLookup("boolean"), $1, NULL, $3, NULL, "!=");}
    | expr EQ_ NULL_        {$$ = makeNode(OPERATOR, TTLookup("boolean"), $1, NULL, $3, NULL, "==");}
    | expr AND_ expr		{$$ = makeNode(OPERATOR, TTLookup("boolean"), $1, NULL, $3, NULL, "AND");}
    | expr OR_ expr			{$$ = makeNode(OPERATOR, TTLookup("boolean"), $1, NULL, $3, NULL, "OR");}
    | NOT_ expr				{$$ = makeNode(OPERATOR, TTLookup("boolean"), $2, NULL, NULL, NULL, "NOT");}
    | '(' expr ')' 			{$$ = $2;}
    | id					{$$ = $1;}
    | NUM_					{$$ = $1;}

stringExpr: expr
          | TEXT_

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
        }
    }
    | ID_ '[' expr ']' {
        // Error if expr is boolean
        if (!typeCheckInt($3)) {
            printf("Line %d: Invalid index\n", yylineno);
            exit(1);
        }

        // Lookup in LST then GST
        struct GST_Node *g_entry = GSTLookup($1->name);
        struct LST_Node *l_entry = LSTLookup(lst, $1->name);
        
        if (g_entry == NULL && l_entry == NULL) {
            printf("Line %d: Array \"%s\" not declared\n", yylineno, $1->name);
            exit(1);
        }
        
        struct Typetable *arr_type;
        if (l_entry) {
            arr_type = l_entry->type;
        } else {
            if (g_entry->typeofvar != ARRAY) {
                printf("Line %d: \"%s\" is not an array\n", yylineno, $1->name);
                exit(1);
            }
            arr_type = g_entry->type;
        }

        $1->type = arr_type;
        $$ = makeArrayLeafNode($1->name, $3, "ARRAY");
        $$->type = arr_type;
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
        if (curr->Phead != NULL) {
            printf("Line %d: Wrong arguments in \"%s\", does not match with declaration\n", yylineno, $1->name);
            exit(1);
        }
        $1->nodetype = FUNCTIONCALL;
        $1->type = curr->type;
        $1->arg_list = NULL;
        $$ = $1;
    }
    // NEW: array.field access grammar
    | ID_ '[' expr ']' '.' ID_ { 
        // Semantic check logic duplicated from below, applied to the array element.
        // Array Identifier part: $1
        // Index Expression: $3
        // Field Identifier: $6
        
        struct GST_Node *g_entry = GSTLookup($1->name);
        struct LST_Node *l_entry = LSTLookup(lst, $1->name);
        
        // Find the type of the UDT array itself (e.g., 'List')
        struct Typetable *TT = g_entry ? g_entry->type : (l_entry ? l_entry->type : NULL);

        if (!TT) { 
            printf("Line %d: Array variable \"%s\" not declared\n", yylineno, $1->name); 
            exit(1); 
        }
        
        // Check if the field exists in the array element's type
        struct FieldlistNode *f = TTLookupField(TT, $6->name);
        if (f == NULL) { 
            printf("Line %d: Field \"%s\" not found in type \"%s\"\n", yylineno, $6->name, TT->name); 
            exit(1); 
        }
        
        // Build AST structure: FIELD node's left child is the ARRAY node.
        struct AST_Node *arr_node = makeArrayLeafNode($1->name, $3, "ARRAY");
        arr_node->type = TT; 
        
        $6->type = f->type; 
        
        $$ = makeNode(FIELD, f->type, arr_node, NULL, $6, NULL, "FIELD");
    }
    | Field


//---------------------------------FIELD---------------------------------------------
Field:  Field '.' ID_ {
            // Recursive field access, works for arr[i].field.subfield as well
            struct FieldlistNode *f = TTLookupField($1->type, $3->name);
            if (f == NULL) {
                printf("Line %d: Field \"%s\" not found in type \"%s\"\n", yylineno, $3->name, $1->type->name);
                exit(1);
            }
            $3->type = f->type;
            $$ = makeNode(FIELD, f->type, $1, NULL, $3, NULL, "FIELD");
        }
        | ID_ '.' ID_ {
            struct Typetable *TT = NULL;
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

            struct FieldlistNode *f = TTLookupField(TT, $3->name);
            if (f == NULL) {
                printf("Line %d: Field \"%s\" not found in type \"%s\"\n", yylineno, $3->name, TT->name);
                exit(1);
            }
            $1->type = TT;
            $3->type = f->type;
            $$ = makeNode(FIELD, f->type, $1, NULL, $3, NULL, "FIELD");
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
    yyin = fopen(fname, "r");
    if (!yyin) {
        perror("Error opening input file");
        return 1;
    }

    ft = fopen("output.xsm", "w");
    if (!ft) {
        perror("Error creating output file");
        fclose(yyin);
        return 1;
    }

    yyparse();

    fclose(yyin);
    fclose(ft);
    return 0;
}
