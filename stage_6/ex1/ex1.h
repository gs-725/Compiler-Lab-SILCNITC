#ifndef _EX1_H
#define _EX1_H

#include <stdio.h>
#include <string.h>

typedef enum Nodetype
{
    VARIABLE,
    ARRAY,
    CONSTANT,
    READ,
    WRITE,
    CONNECTOR,
    OPERATOR,
    WHILE,
    IF,
    BREAK,
    CONTINUE,
    BREAKPOINT,
    FUNCTION,
    BODY,
    RET,
    FUNCTIONCALL,
    NULL__,
    FIELD,
    INITIALIZE,
    ALLOC,
    FREE,
} Nodetype;

//-------------------Type Table-------------------
struct Typetable
{
    char *name;               // name of the type
    int size;                 // size of the type
    struct Fieldlist *fields; // pointer to the field list of structures
    struct Typetable *next;   // pointer to next type table entry
};

void TTCreate();
struct Typetable *TTLookup(char *);
struct Typetable *TTInstall(char *, int, struct Fieldlist *);
int TTGetSize(struct Typetable *);
struct FieldlistNode *TTLookupField(struct Typetable *, char *);
void TTPrint();

//-------------------Field List-------------------
struct FieldlistNode
{
    char *name;
    int fieldIndex;
    struct Typetable *type;
    struct FieldlistNode *next;
};

struct Fieldlist
{
    struct FieldlistNode *head, *tail;
    int size;
};

struct Fieldlist *FLInit();
struct FieldlistNode *FLInitNode(char *, int, struct Typetable *);
struct Fieldlist *FLInstall(struct Fieldlist *, char *, struct Typetable *);
void FLPrint(struct Fieldlist *);

//-------------------AST-------------------
struct AST_Node
{
    int val;                             // value of a number for NUM nodes.
    struct Typetable *type;              // type of variable
    char *name;                          // name of a variable for ID nodes
    Nodetype nodetype;                   // type of node
    char *s;                             // string representation of the node
    struct GST_Node *GSTentry;           // Pointer to Global Symbol Table entry for variables
    struct AST_Node *next_arg;           // Pointer to next argument in the argument list
    struct AST_Node *arg_list;           // Pointer to the argument list
    struct AST_Node *left, *mid, *right; // left, middle and right branches
};

struct AST_Node *makeVariableLeafNode(char *, char *);
struct AST_Node *makeConstantLeafNode(struct Typetable *, int, char *);
struct AST_Node *makeArrayLeafNode(char *, struct AST_Node *, char *);
struct AST_Node *makeNode(Nodetype, struct Typetable *, struct AST_Node *, struct AST_Node *, struct AST_Node *, struct GST_Node *, char *);
void ASTPrint(struct AST_Node *);

//-------------------Global Symbol Table-------------------
struct GST_Node
{
    char *name;              // name of the variable
    struct Typetable *type;  // type of the variable
    int size;                // size of the type
    Nodetype typeofvar;      // variable, array or function
    int binding;             // stores the static memory address allocated to the variable
    struct ParamNode *Phead; // stores the parameter list for functions
    struct LSTable *LST;     // stores the local symbol table for functions
    int flabel;              // a label for identifying the starting address of a function's code
    struct GST_Node *next;   // pointer to next symbol table entry
};

struct GST_Node *GSTLookup(char *);
struct GST_Node *GSTInstall(char *, struct Typetable *, int, Nodetype, struct ParamNode *);
void GSTChangeType(struct AST_Node *, struct Typetable *);
void GSTPrint();

//-------------------Local Symbol Table-------------------
struct LST_Node
{
    char *name;             // name of the variable
    struct Typetable *type; // type of the variable
    int binding;            // stores the static memory address allocated to the variable
    struct LST_Node *next;  // pointer to next symbol table entry
};

struct LSTable
{
    struct LST_Node *head;
    struct LST_Node *tail;
    int size;
};

struct LST_Node *LSTInitNode(char *, struct Typetable *, int);
struct LSTable *LSTInitTable();
struct LST_Node *LSTLookup(struct LSTable *, char *);
struct LSTable *LSTInstall(struct LSTable *, char *, struct Typetable *);
struct LSTable *LSTDelete(struct LSTable *);
struct LSTable *LSTCopy(struct LSTable *);
void LSTChangeType(struct LSTable *, struct AST_Node *, struct Typetable *);
void LSTPrint(struct LSTable *);

//-------------------Parameter List-------------------
struct ParamNode
{
    char *name;             // name of the variable
    struct Typetable *type; // type of the variable
    Nodetype typeofvar;     // variable, array or function
    struct ParamNode *next; // pointer to next symbol table entry
};

struct ParamNode *ParamInstall(struct ParamNode *, char *, struct Typetable *, Nodetype);
struct ParamNode *ParamDelete(struct ParamNode *);
struct ParamNode *ParamCopy(struct ParamNode *);
void ParamPrint(struct ParamNode *);
int ParamGetCount(struct ParamNode *);
int ParamCheck(struct ParamNode *, struct ParamNode *);

//-------------------Argument List-------------------
struct AST_Node *ASTArgAppend(struct AST_Node *, struct AST_Node *);
int checkASTParam(struct ParamNode *, struct AST_Node *);
struct AST_Node *ParamToArg(struct ParamNode *);
struct ParamNode *ArgToParam(struct AST_Node *);
void ArgDelete(struct AST_Node *);

//-------------------Code Generation-------------------
int codeGen(struct AST_Node *, struct LSTable *, FILE *);

//-------------------Auxiliary Functions-------------------
int getAddr(struct AST_Node *, struct LSTable *, FILE *);
int getArrayAddr(struct AST_Node *, struct LSTable *, FILE *);
int getFieldAddr(struct AST_Node *, struct LSTable *, FILE *);

int pushArgs(struct AST_Node *, int, struct LSTable *, FILE *);
struct LSTable *LSTParamInstall(struct LSTable *, struct ParamNode *);
void generateHeader(FILE *);

struct AST_Node *ASTChangeTypeGST(struct AST_Node *, struct Typetable *);
struct AST_Node *ASTChangeTypeLST(struct LSTable *, struct AST_Node *, struct Typetable *);

//-------------------Type Checking-------------------
int typeCheckInt(struct AST_Node *);
int typeCheckBool(struct AST_Node *);
int typeCheckStr(struct AST_Node *);
int typeCheckVoid(struct AST_Node *);

//-------------------Static Storage Allocation-------------------
int allocate(int);
int getSP();

//-------------------Register Allocation-------------------
int getReg();
void freeReg();

//-------------------Label Allocation-------------------
int getLabel();
int getFlabel(char name);
int setFlabel();

#endif
