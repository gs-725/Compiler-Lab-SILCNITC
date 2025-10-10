#ifndef EX1_H
#define EX1_H

#include <stdio.h>
#include <string.h>

typedef enum Nodetype
{
    VARIABLE,
    CONSTANT,
    READ,
    WRITE,
    CONNECTOR,
    OPERATOR,
    WHILE,
    IF,
    BREAK,
    REPEAT,
    DOWHILE,
    CONTINUE,
    ARRAY,
    ADDRESS,
    POINTER,
    FUNCTION,
    BODY,
    RET,
    FUNCTIONCALL,
} Nodetype;

typedef enum Type
{
    INTEGER,
    BOOLEAN,
    STRING,
    VOID,
    PVOID,
    POINTER_TO_INTEGER, 
    POINTER_TO_STRING,
} Type;

//-------------------AST-------------------
struct AST_Node
{
    int val;                             // value of a number for NUM nodes.
    Type type;                           // type of variable
    char *name;                          // name of a variable for ID nodes
    Nodetype nodetype;                   // type of node
    char *s;                             // string representation of the node
    struct GST_Node *GSTentry;           // Pointer to Global Symbol Table entry for variables
    struct AST_Node *next_arg;           // Pointer to next argument in the argument list
    struct AST_Node *arg_list;           // Pointer to the argument list
    struct AST_Node *left, *mid, *right; // left, middle and right branches
};

struct AST_Node *makeVariableLeafNode(char *, char *);
struct AST_Node *makeConstantLeafNode(Type, int, char *);
struct AST_Node *makeArrayLeafNode(char *, struct AST_Node *, char *);
struct AST_Node *makeNode(Nodetype, Type, struct AST_Node *, struct AST_Node *, struct AST_Node *, struct GST_Node *, char *);
struct AST_Node *makePointerNode(Nodetype node_type, struct AST_Node *l, char *s); 
void ASTPrint(struct AST_Node *);

//-------------------Global Symbol Table-------------------
struct GST_Node
{
    char *name;              // name of the variable
    Type type;               // type of the variable
    int size;                // size of the type
    Nodetype typeofvar;      // variable, array or function
    int binding;             // stores the static memory address allocated to the variable
    int ptr_type;
    struct ParamNode *Phead; // stores the parameter list for functions
    struct LSTable *LST;     // stores the local symbol table for functions
    int flabel;              // a label for identifying the starting address of a function's code
    struct GST_Node *next;   // pointer to next symbol table entry
};

struct GST_Node *GSTLookup(char *);
struct GST_Node *GSTInstall(char *, Type, int, Nodetype, struct ParamNode *);
void GSTChangeType(struct AST_Node *, Type);
void GSTPrint();

//-------------------Local Symbol Table-------------------
struct LST_Node
{
    char *name;            // name of the variable
    Type type;             // type of the variable
    int binding;           // stores the static memory address allocated to the variable
    struct LST_Node *next; // pointer to next symbol table entry
};

struct LSTable
{
    struct LST_Node *head;
    struct LST_Node *tail;
    int size;
};

struct LST_Node *LSTInitNode(char *, Type, int);
struct LSTable *LSTInitTable();
struct LST_Node *LSTLookup(struct LSTable *, char *);
struct LSTable *LSTInstall(struct LSTable *, char *, Type);
struct LSTable *LSTDelete(struct LSTable *);
struct LSTable *LSTCopy(struct LSTable *);
void LSTChangeType(struct LSTable *, struct AST_Node *, Type);
void LSTPrint(struct LSTable *);

//-------------------Parameter List-------------------
struct ParamNode
{
    char *name;             // name of the variable
    Type type;              // type of the variable
    Nodetype typeofvar;     // variable, array or function
    struct ParamNode *next; // pointer to next symbol table entry
};

struct ParamNode *ParamInstall(struct ParamNode *, char *, Type, Nodetype);
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
int pushArgs(struct AST_Node *, int, struct LSTable *, FILE *);
struct LSTable *LSTParamInstall(struct LSTable *, struct ParamNode *);
void generateHeader(FILE *);
void printIndent(int, int);
void print_tree(struct AST_Node *, int, int);

struct AST_Node *ASTChangeTypeGST(struct AST_Node *, Type);
struct AST_Node *ASTChangeTypeLST(struct LSTable *, struct AST_Node *, Type);

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
