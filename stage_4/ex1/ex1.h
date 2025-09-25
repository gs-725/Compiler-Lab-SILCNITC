#ifndef EX1_H
#define EX1_H

#include <stdlib.h>
#include <stdio.h>


typedef enum Nodetype
{
    VARIABLE,
    CONSTANT,
    READ,
    WRITE,
    STATEMENT,
    OPERATOR,
    WHILE,
    IF,
    BREAK,
    CONTINUE,
    REPEAT,
    DOWHILE,
    ARRAY,
} Nodetype;

typedef enum Type
{
    INTEGER,
    BOOLEAN,
    VOID,
    STRING,
} Type;

typedef struct GST_Node
{
    char *name;
    int type;
    int size;
    int size2;
    int binding;
    int dimensions;
    struct GST_Node *next;
} GST_Node;


typedef struct AST_Node
{
    int val;
    Type type;
    char *varname;
    Nodetype nodetype;
    struct GST_Node *GSTentry;
    char *s;
    struct AST_Node *left, *mid, *right;
} AST_Node;


struct AST_Node *makeConstLeafNode(Type, int, char *);
struct AST_Node *makeVarLeafNode(char *, char *);
struct AST_Node *makeNode(Nodetype, Type, struct AST_Node *, struct AST_Node *, struct AST_Node *, char *);
struct AST_Node *makeArrLeafNode(char *, struct AST_Node *, char *);

struct GST_Node *GSTLookup(char *);
struct GST_Node *GSTInstall(char *, Type, int size, int size2, int dimensions);
void GSTChangeType(struct AST_Node *, Type);
void GSTPrint();

void printIndent(int, int);
void print_tree(struct AST_Node *, int, int);
int getReg();
void freeReg();
int getLabel();
int getAddr(struct AST_Node *t);
int codeGen(struct AST_Node *, FILE *);
void xsmgenerator(struct AST_Node *t);

#endif 
