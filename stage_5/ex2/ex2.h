#ifndef EX2_H
#define EX2_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h> // Added for clarity
extern int yylineno;
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
    TUPLE_FIELD_ACCESS, // New AST node type for a.name
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
    TUPLE, // New type for tuple variables
} Type;

// Forward declarations
struct TST_Node;
struct ParamNode;
struct LSTable;

struct AST_Node
{
    int val;
    Type type;
    char *name;
    Nodetype nodetype;
    char *s;
    struct GST_Node *GSTentry;
    struct TST_Node *TSTentry; // New: Pointer to TST entry for tuple variables/fields
    struct AST_Node *next_arg;
    struct AST_Node *arg_list;
    struct AST_Node *left, *mid, *right;
};

// Global Symbol Table Node (GST) structure
struct GST_Node
{
    char *name;
    Type type;
    int size;
    Nodetype typeofvar;
    int binding;
    int ptr_type;
    struct ParamNode *Phead;
    struct LSTable *LST;
    int flabel;
    struct TST_Node *TSType; // New: Pointer to TST entry if type is TUPLE
    struct GST_Node *next;
};

// Local Symbol Table Node (LST) structure
struct LST_Node
{
    char *name;
    Type type;
    int binding;
    struct TST_Node *TSType; // New: Pointer to TST entry if type is TUPLE
    struct LST_Node *next;
};

struct LSTable
{
    struct LST_Node *head;
    struct LST_Node *tail;
    int size;
};

// Tuple Symbol Table Node (TST) structure
struct TST_Node
{
    char *name; // Name of the tuple type (e.g., "student")
    int size;   // Total size of the tuple (number of fields/words)
    struct ParamNode *Fhead; // Field list (reusing ParamNode: name=field name, type=field type)
    struct TST_Node *next;
};

struct ParamNode
{
    char *name;
    Type type;
    Nodetype typeofvar;
    struct ParamNode *next;
};


// --- AST Node Constructors ---
struct AST_Node *makeVariableLeafNode(char *, char *);
struct AST_Node *makeConstantLeafNode(Type, int, char *);
struct AST_Node *makeArrayLeafNode(char *, struct AST_Node *, char *);
struct AST_Node *makeNode(Nodetype, Type, struct AST_Node *, struct AST_Node *, struct AST_Node *, struct GST_Node *, char *);
struct AST_Node *makePointerNode(Nodetype node_type, struct AST_Node *l, char *s); 
struct AST_Node *makeTupleFieldAccessNode(struct AST_Node *tuple_var_node, char *field_name, char *s); // New

// --- Global Symbol Table (GST) Functions ---
struct GST_Node *GSTLookup(char *);
struct GST_Node *GSTInstall(char *, Type, int, Nodetype, struct ParamNode *, struct TST_Node *); // Modified
void GSTChangeType(struct AST_Node *, Type, struct TST_Node *); // Modified
void GSTPrint();

// --- Local Symbol Table (LST) Functions ---
struct LST_Node *LSTInitNode(char *, Type, int, struct TST_Node *); // Modified
struct LSTable *LSTInitTable();
struct LST_Node *LSTLookup(struct LSTable *, char *);
struct LSTable *LSTInstall(struct LSTable *, char *, Type, struct TST_Node *); // Modified
struct LSTable *LSTDelete(struct LSTable *);
struct LSTable *LSTCopy(struct LSTable *);
void LSTChangeType(struct LSTable *, struct AST_Node *, Type, struct TST_Node *); // Modified
void LSTPrint(struct LSTable *);
struct LSTable *LSTParamInstall(struct LSTable *, struct ParamNode *);

// --- Tuple Symbol Table (TST) Functions ---
struct TST_Node *TSTLookup(char *name);
struct TST_Node *TSTInstall(char *name, struct ParamNode *Fhead);
struct ParamNode *TSTGetField(struct TST_Node *tst, char *field_name);
int TSTGetFieldOffset(struct TST_Node *tst, char *field_name);
int TSTGetTupleSize(char *name);
void TSTPrint();

// --- Parameter List Functions ---
struct ParamNode *ParamInstall(struct ParamNode *, char *, Type, Nodetype);
struct ParamNode *ParamDelete(struct ParamNode *);
struct ParamNode *ParamCopy(struct ParamNode *);
void ParamPrint(struct ParamNode *);
int ParamGetCount(struct ParamNode *);
int ParamCheck(struct ParamNode *, struct ParamNode *);

// --- AST/Argument Functions ---
struct AST_Node *ASTArgAppend(struct AST_Node *, struct AST_Node *);
int checkASTParam(struct ParamNode *, struct AST_Node *);
struct AST_Node *ParamToArg(struct ParamNode *);
struct ParamNode *ArgToParam(struct AST_Node *);
void ArgDelete(struct AST_Node *);

// --- Code Generation & Utility ---
int codeGen(struct AST_Node *, struct LSTable *, FILE *);
int getAddr(struct AST_Node *, struct LSTable *, FILE *);
int getArrayAddr(struct AST_Node *, struct LSTable *, FILE *);
int getFieldAddr(struct AST_Node *t, struct LSTable *LST, FILE *target_file); // New
int pushArgs(struct AST_Node *, int, struct LSTable *, FILE *);
void generateHeader(FILE *);
void printIndent(int, int);
void print_tree(struct AST_Node *, int, int);

struct AST_Node *ASTChangeTypeGST(struct AST_Node *, Type, struct TST_Node *);
struct AST_Node *ASTChangeTypeLST(struct LSTable *, struct AST_Node *, Type, struct TST_Node *);

int allocate(int);
int getSP();

int getReg();
void freeReg();

int getLabel();
int getFLabel(char *name);
int setFLabel();
void localprint();
#endif
