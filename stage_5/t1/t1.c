#include "t1.h"
#include <string.h>
#include <stdlib.h>

struct GST_Node *Ghead = NULL;

//------------------------Static Storage Allocation------------------------
int SP = 4096;

int allocate(int size)
{
    int memAddr = SP;
    SP += size;
    return memAddr;
}

int getSP()
{
    return SP;
}

//------------------------Static Storage Allocation------------------------

//---------------------------Register Allocation-----------------------------
int free_reg = -1;

int getReg()
{
    free_reg++;
    return free_reg;
}

void freeReg()
{
    free_reg--;
}
//---------------------------Register Allocation-----------------------------

//------------------------Label Allocation----------------------------

int label = 0;
int fLabel = 1;

int setFLabel()
{
    return fLabel++;
}

int getFLabel(char *name)
{
    struct GST_Node *gst = Ghead;
    while (gst)
    {
        if (strcmp(gst->name, name) == 0)
            if (gst->flabel != -1)
                return gst->flabel;
            else
            {
                printf("Error: Function \"%s\" declared as a variable\n", name);
                exit(1);
            }
        gst = gst->next;
    }
    printf("Error: Function \"%s\" not declared\n", name);
}

int getLabel()
{
    return label++;
}

//------------------------Label Allocation----------------------------

//---------------------------Parameter List-----------------------------
struct ParamNode *ParamInstall(struct ParamNode *Phead, char *name, Type type, Nodetype typeofvar)
{
    struct ParamNode *new_node = (struct ParamNode *)malloc(sizeof(struct ParamNode));
    new_node->name = name;
    new_node->type = type;
    new_node->typeofvar = typeofvar;
    new_node->next = NULL;
    if (Phead == NULL)
        return new_node;
    struct ParamNode *temp = Phead;
    while (temp->next != NULL)
        temp = temp->next;
    temp->next = new_node;
    return Phead;
}

struct ParamNode *ParamDelete(struct ParamNode *Phead)
{
    struct ParamNode *temp = Phead;
    while (temp != NULL)
    {
        Phead = Phead->next;
        free(temp);
        temp = Phead;
    }
    return NULL;
}

struct ParamNode *ParamCopy(struct ParamNode *Phead)
{
    struct ParamNode *new_head = NULL;
    struct ParamNode *temp = Phead;
    while (temp != NULL)
    {
        new_head = ParamInstall(new_head, temp->name, temp->type, temp->typeofvar);
        temp = temp->next;
    }
    return new_head;
}

void ParamPrint(struct ParamNode *Phead)
{
    struct ParamNode *temp = Phead;
    while (temp != NULL)
    {
        printf("%s\t", temp->name);
        if (temp->type == 0)
            printf("int\t");
        else if (temp->type == 2)
            printf("str\t");
        temp = temp->next;
    }
    printf("\n");
}

int ParamGetCount(struct ParamNode *Phead)
{
    int count = 0;
    struct ParamNode *temp = Phead;
    while (temp != NULL)
    {
        count++;
        temp = temp->next;
    }
    return count;
}

int ParamCheck(struct ParamNode *Phead1, struct ParamNode *Phead2)
{
    if (ParamGetCount(Phead1) != ParamGetCount(Phead2))
        return 0;
    struct ParamNode *temp1 = Phead1;
    struct ParamNode *temp2 = Phead2;
    while (temp1 != NULL && temp2 != NULL)
    {
        if (temp1->type != temp2->type || temp1->typeofvar != temp2->typeofvar)
            return 0;
        temp1 = temp1->next;
        temp2 = temp2->next;
    }
    if (temp1 == NULL && temp2 == NULL)
        return 1;
    return 0;
}
//---------------------------Parameter List-----------------------------

//---------------------------LST-----------------------------
struct LST_Node *LSTInitNode(char *name, Type type, int binding)
{
    struct LST_Node *new_node = (struct LST_Node *)malloc(sizeof(struct LST_Node));
    new_node->name = name;
    new_node->type = type;
    new_node->binding = binding;
    new_node->next = NULL;
    return new_node;
}

struct LSTable *LSTInitTable()
{
    struct LSTable *new_table = (struct LSTable *)malloc(sizeof(struct LSTable));
    new_table->head = NULL;
    new_table->tail = NULL;
    new_table->size = 0;
    return new_table;
}

struct LST_Node *LSTLookup(struct LSTable *table, char *name)
{
    struct LST_Node *temp = table->head;
    while (temp)
    {
        if (strcmp(temp->name, name) == 0)
            return temp;
        temp = temp->next;
    }
    return NULL;
}

struct LSTable *LSTInstall(struct LSTable *table, char *name, Type type)
{
    struct LST_Node *new_node = LSTLookup(table, name);
    if (new_node != NULL)
    {
        printf("Error: Variable \"%s\" redeclared (LST)\n", name);
        exit(1);
    }
    table->size++;
    if (table->head == NULL)
    {
        table->head = table->tail = LSTInitNode(name, type, table->size);
    }
    else
    {
        table->tail->next = LSTInitNode(name, type, table->size);
        table->tail = table->tail->next;
    }
    return table;
}

struct LSTable *LSTDelete(struct LSTable *table)
{
    struct LST_Node *temp = table->head;
    while (temp)
    {
        struct LST_Node *temp2 = temp;
        temp = temp->next;
        free(temp2);
    }
    table = LSTInitTable();
    return table;
}

struct LSTable *LSTCopy(struct LSTable *table)
{
    struct LSTable *new_table = LSTInitTable();
    struct LST_Node *temp = table->head;
    while (temp)
    {
        if (new_table->head == NULL)
        {
            new_table->head = new_table->tail = LSTInitNode(temp->name, temp->type, temp->binding);
        }
        else
        {
            new_table->tail->next = LSTInitNode(temp->name, temp->type, temp->binding);
            new_table->tail = new_table->tail->next;
        }
        temp = temp->next;
    }
    new_table->size = table->size;
    return new_table;
}

void LSTChangeType(struct LSTable *table, struct AST_Node *root, Type type)
{
    struct LST_Node *temp = LSTLookup(table, root->name);
    if (temp == NULL)
    {
        printf("Error: Variable \"%s\" undeclared (LST)\n", root->name);
        exit(1);
    }
    temp->type = type;
}

void LSTPrint(struct LSTable *table)
{
    struct LST_Node *temp = table->head;
    printf("LST:\n");
    while (temp)
    {
        printf("%s\t", temp->name);
        if (temp->type == 0)
            printf("int\t");
        else if (temp->type == 2)
            printf("str\t");
        printf("%d\t", temp->binding);
        temp = temp->next;
        printf("\n");
    }
}
//---------------------------LST-----------------------------

//---------------------------GST-----------------------------
struct GST_Node *GSTLookup(char *name)
{
    struct GST_Node *temp = Ghead;
    while (temp != NULL)
    {
        if (strcmp(temp->name, name) == 0)
            return temp;
        temp = temp->next;
    }
    return NULL;
}

struct GST_Node *GSTInstall(char *name, Type type, int size, Nodetype typeofvar, struct ParamNode *Phead)
{
    struct GST_Node *new_node = GSTLookup(name);
    if (new_node != NULL)
    {
        printf("Variable \"%s\" already declared\n", name);
        exit(1);
    }
    new_node = (struct GST_Node *)malloc(sizeof(struct GST_Node));
    new_node->name = name;
    new_node->type = type;
    new_node->size = size;
    new_node->typeofvar = typeofvar;
    if (typeofvar == FUNCTION)
    {
        new_node->binding = -1;
        if (strcmp(name, "main") == 0)
            new_node->flabel = 0;
        else
            new_node->flabel = setFLabel();
    }
    else
    {
        new_node->binding = allocate(size);
        new_node->flabel = -1;
    }
    new_node->Phead = Phead;
    if (Ghead == NULL)
    {
        Ghead = new_node;
    }
    else
    {
        struct GST_Node *temp = Ghead;
        while (temp->next != NULL)
            temp = temp->next;
        temp->next = new_node;
    }
    return Ghead;
}

void GSTChangeType(struct AST_Node *root, Type type)
{
    struct GST_Node *temp = GSTLookup(root->name);
    if (temp == NULL)
    {
        printf("Variable \"%s\" not declared\n", root->name);
        exit(1);
    }
    temp->type = type;
}

void GSTPrint()
{
    char *type;
    char *typeofvar;
    struct GST_Node *temp = Ghead;
    printf("%-12s\t%-5s\t%-5s\t%-10s\t%-7s\n", "Name", "Type", "Size", "TypeofVar", "Binding");
    while (temp != NULL)
    {
        if (temp->type == INTEGER)
        {
            type = (char *)malloc(sizeof(char) * 4);
            strcpy(type, "int");
        }
        else if (temp->type == STRING)
        {
            type = (char *)malloc(sizeof(char) * 4);
            strcpy(type, "str");
        }

        if (temp->typeofvar == ARRAY)
        {
            typeofvar = (char *)malloc(sizeof(char) * 13);
            strcpy(typeofvar, "array");
        }
        else if (temp->typeofvar == VARIABLE)
        {
            typeofvar = (char *)malloc(sizeof(char) * 13);
            strcpy(typeofvar, "variable");
        }
        else if (temp->typeofvar == FUNCTION)
        {
            typeofvar = (char *)malloc(sizeof(char) * 13);
            strcpy(typeofvar, "function");
        }

        printf("%-12s\t%-5s\t%-5d\t%-10s\t%-7d\n", temp->name, type, temp->size, typeofvar, temp->binding);
        temp = temp->next;
    }
}
void localprint(){
	printf("Press 1 if you want to print the lst  of a function to exit press 0\n");
	int temp=-1;
	scanf("%d",&temp);
	
	while(1){
		if(temp==0)
			return;
		printf("Enter the function name:");
		char funcname[256];
		scanf("%s",funcname);
		struct GST_Node* node=GSTLookup(funcname);
		if(node!=NULL){
			if(node->typeofvar==FUNCTION){
				LSTPrint(node->LST);
				
				
							}
			else
				printf("No function exists with given name");
				
				}
		else
			printf("No function exists with given name");
		printf("Press 1 if you want to print the lst of a function to exit press 0\n");
		scanf("%d",&temp);
			
		}
		 }
//---------------------------GST-----------------------------

//---------------------------AST-----------------------------
struct AST_Node *makeVariableLeafNode(char *name, char *s)
{
    struct AST_Node *new_node = (struct AST_Node *)malloc(sizeof(struct AST_Node));
    new_node->s = (char *)malloc(sizeof(char) * strlen(s));
    new_node->s = strdup(s);
    new_node->nodetype = VARIABLE;
    new_node->name = (char *)malloc(sizeof(char) * strlen(name));
    new_node->name = strdup(name);
    new_node->GSTentry = GSTLookup(name);
    if (new_node->GSTentry)
        new_node->type = new_node->GSTentry->type;
    new_node->left = (struct AST_Node *)NULL;
    new_node->right = (struct AST_Node *)NULL;
    new_node->mid = (struct AST_Node *)NULL;
    return new_node;
}

struct AST_Node *makeConstantLeafNode(Type type, int val, char *s)
{
    struct AST_Node *new_node = (struct AST_Node *)malloc(sizeof(struct AST_Node));
    new_node->s = (char *)malloc(sizeof(char) * strlen(s));
    new_node->s = strdup(s);
    new_node->nodetype = CONSTANT;
    new_node->type = type;
    new_node->val = val;
    new_node->GSTentry = (struct GST_Node *)NULL;
    new_node->left = (struct AST_Node *)NULL;
    new_node->right = (struct AST_Node *)NULL;
    new_node->mid = (struct AST_Node *)NULL;
    new_node->name = (char *)NULL;
    return new_node;
}

struct AST_Node *makeArrayLeafNode(char *name, struct AST_Node *l, char *s)
{
    struct AST_Node *new_node = (struct AST_Node *)malloc(sizeof(struct AST_Node));
    new_node->s = (char *)malloc(sizeof(char) * strlen(s));
    new_node->s = strdup(s);
    new_node->nodetype = ARRAY;
    new_node->name = (char *)malloc(sizeof(char) * strlen(name));
    new_node->name = strdup(name);
    new_node->GSTentry = GSTLookup(name);
    new_node->type = new_node->GSTentry->type;
    new_node->left = l;
    new_node->right = (struct AST_Node *)NULL;
    new_node->mid = (struct AST_Node *)NULL;
    return new_node;
}

struct AST_Node *makeNode(Nodetype node_type, Type type, struct AST_Node *l, struct AST_Node *m, struct AST_Node *r, struct GST_Node *gst, char *s)
{
    struct AST_Node *new_node = (struct AST_Node *)malloc(sizeof(struct AST_Node));

    // Type checking
    if (node_type == OPERATOR && type == INTEGER)
    {
        if (l->type != INTEGER || r->type != INTEGER)
        {
            printf("Error: Type mismatch. \"Operand is not integer\".\n");
            exit(1);
        }
    }

    if (node_type == WHILE || node_type == IF)
    {
        if (l->type != BOOLEAN)
        {
            printf("Error: Type mismatch. \"Guard is not boolean\".\n");
            exit(1);
        }
    }

    new_node->s = (char *)malloc(sizeof(char) * strlen(s));
    new_node->s = strdup(s);
    new_node->nodetype = node_type;
    new_node->type = type;
    new_node->left = l;
    new_node->mid = m;
    new_node->right = r;
    new_node->GSTentry = gst;
    new_node->name = (char *)NULL;
    return new_node;
}

struct AST_Node *ASTChangeTypeGST(struct AST_Node *root, Type type)
{
    if (root != NULL)
    {
        root->left = ASTChangeTypeGST(root->left, type);
        root->right = ASTChangeTypeGST(root->right, type);
        if (root->nodetype == VARIABLE)
        {
            GSTChangeType(root, type);
        }
        root = NULL;
        free(root);
    }
}

struct AST_Node *ASTChangeTypeLST(struct LSTable *table, struct AST_Node *root, Type type)
{
    if (root != NULL)
    {
        root->left = ASTChangeTypeLST(table, root->left, type);
        root->right = ASTChangeTypeLST(table, root->right, type);
        if (root->nodetype == VARIABLE)
        {
            LSTChangeType(table, root, type);
        }
        root = NULL;
        free(root);
    }
}

void ASTPrint(struct AST_Node *root)
{
    if (root->nodetype == VARIABLE)
    {
        printf("%s", root->name);
    }

    else if (root->nodetype == CONSTANT)
    {
        if (root->type == INTEGER)
            printf("%d", root->val);
        else if (root->type == STRING)
            printf("%s", root->s);
    }

    else if (root->nodetype == READ)
    {
        printf("read(");
        ASTPrint(root->left);
        printf(")");
    }

    else if (root->nodetype == WRITE)
    {
        printf("write(");
        ASTPrint(root->left);
        printf(")");
    }

    else if (root->nodetype == CONNECTOR)
    {
        ASTPrint(root->left);
        printf(";\n");
        ASTPrint(root->right);
    }

    else if (root->nodetype == OPERATOR)
    {
        if (root->type == BOOLEAN)
        {
            ASTPrint(root->left);
            printf("%s", root->s);
            ASTPrint(root->right);
        }
        else
        {
            ASTPrint(root->left);
            printf("%s", root->s);
            ASTPrint(root->right);
        }
    }

    else if (root->nodetype == WHILE)
    {
        printf("while(");
        ASTPrint(root->left);
        printf(")\n");
        ASTPrint(root->right);
    }

    else if (root->nodetype == IF)
    {
        printf("if(");
        ASTPrint(root->left);
        printf(")\n");
        ASTPrint(root->mid);
        printf(";\n");
        if (root->right != NULL)
        {
            printf("else\n");
            ASTPrint(root->right);
        }
    }

    else if (root->nodetype == ARRAY)
    {
        printf("%s[", root->name);
        ASTPrint(root->left);
        printf("]");
    }

    else if (root->nodetype == FUNCTION)
    {
        printf("%s(", root->GSTentry->name);
        struct AST_Node *curr = root->right;
        while (curr)
        {
            printf("%s", curr->s);
            curr = curr->next_arg;
            if (curr)
                printf(", ");
        }
        if (root->left != NULL)
        {
            printf(") {\n");
            ASTPrint(root->left);
            printf("}\n");
        }
        else
        {
            struct AST_Node *curr = root->arg_list;
            while (curr)
            {
                printf("%s", curr->s);
                curr = curr->next_arg;
                if (curr)
                    printf(", ");
            }
            printf(")");
        }
    }

    else if (root->nodetype == RET)
    {
        printf("return ");
        ASTPrint(root->left);
        printf(";\n");
    }
}

void printIndent(int depth, int isRight) {
    for (int i = 0; i < depth - 1; i++) {
        printf("    ");
    }
    if (depth > 0) {
        printf(isRight ? "└─ " : "├─ ");
    }
}

void print_tree(struct AST_Node *root, int lvl, int isRight)
{
    if (root == NULL) return;
    printIndent(lvl, isRight);
    printf("%s\n", root->s);
    if (root->left != NULL)
        print_tree(root->left, lvl + 1, 0);
    if (root->right != NULL)
        print_tree(root->right, lvl + 1, 1);
    if (root->mid != NULL)
        print_tree(root->mid, lvl + 1, 1);
}


//---------------------------AST-----------------------------

//---------------------------Argument List-----------------------------
struct AST_Node *ASTArgAppend(struct AST_Node *head, struct AST_Node *arg)
{
    if (head == NULL)
        return arg;
    struct AST_Node *curr = head;
    while (curr->next_arg)
        curr = curr->next_arg;
    curr->next_arg = arg;
    return head;
}

int checkASTParam(struct ParamNode *Phead, struct AST_Node *Ahead)
{
    struct ParamNode *curr_param = Phead;
    struct AST_Node *curr_arg = Ahead;
    while (curr_param && curr_arg)
    {
        if (curr_param->type != curr_arg->type)
            return 0;
        curr_param = curr_param->next;
        curr_arg = curr_arg->next_arg;
    }
    if (curr_param || curr_arg)
        return 0;
    return 1;
}

struct AST_Node *ParamToArg(struct ParamNode *Phead)
{
    struct AST_Node *head = NULL;
    struct ParamNode *curr = Phead;
    while (curr)
    {
        struct AST_Node *new_node = makeNode(VARIABLE, curr->type, NULL, NULL, NULL, NULL, curr->name);
        head = ASTArgAppend(head, new_node);
        curr = curr->next;
    }
    return head;
}

struct ParamNode *ArgToParam(struct AST_Node *Ahead)
{
    struct ParamNode *Phead = NULL;
    struct AST_Node *curr = Ahead;
    while (curr)
    {
        Phead = ParamInstall(Phead, curr->name, curr->type, curr->nodetype);
        curr = curr->next_arg;
    }
    return Phead;
}

void ArgDelete(struct AST_Node *head)
{
    if (head == NULL)
        return;
    ArgDelete(head->next_arg);
    free(head);
}
//---------------------------Argument List-----------------------------

//---------------------------Code Generation-----------------------------
int numWhile = 0;
int labelIn[100], labelOut[100];
int codeGen(struct AST_Node *t, struct LSTable *LST, FILE *target_file) // returns the register number
{
    int p, q, addrReg;

    if (t->nodetype == CONSTANT)
    {
        if (t->type == INTEGER)
        {
            p = getReg();
            fprintf(target_file, "MOV R%d, %d\n", p, t->val);
            return p;
        }
        else if (t->type == STRING)
        {
            p = getReg();
            fprintf(target_file, "MOV R%d, %s\n", p, t->s);
            return p;
        }
    }

    else if (t->nodetype == VARIABLE)
    {
        p = getReg();
        addrReg = getAddr(t, LST, target_file);
        fprintf(target_file, "MOV R%d, [R%d]\n", p, addrReg);
        freeReg();
        return p;
    }

    else if (t->nodetype == READ)
    {
        p = getReg();
        if (t->left->nodetype == VARIABLE)
        {
            addrReg = getAddr(t->left, LST, target_file);
            fprintf(target_file, "MOV R%d, R%d\n", p, addrReg);
            freeReg();
        }
        else if (t->left->nodetype == ARRAY)
        {
            addrReg = getArrayAddr(t->left, LST, target_file);
            fprintf(target_file, "MOV R%d, R%d\n", p, addrReg);
            freeReg();
        }
        q = getReg();
        fprintf(target_file, "MOV R%d, \"Read\"\n", q);
        fprintf(target_file, "PUSH R%d\n", q);
        fprintf(target_file, "MOV R%d, -1\n", q);
        fprintf(target_file, "PUSH R%d\n", q);
        fprintf(target_file, "PUSH R%d\n", p);
        fprintf(target_file, "PUSH R%d\n", q);
        fprintf(target_file, "PUSH R%d\n", q);
        fprintf(target_file, "CALL 0\n");
        fprintf(target_file, "POP R%d\n", q);
        fprintf(target_file, "POP R%d\n", q);
        fprintf(target_file, "POP R%d\n", q);
        fprintf(target_file, "POP R%d\n", q);
        fprintf(target_file, "POP R%d\n", q);
        freeReg();
        freeReg();
        return -1;
    }

    else if (t->nodetype == WRITE)
    {
        p = codeGen(t->left, LST, target_file);
        q = getReg();
        fprintf(target_file, "MOV R%d, \"Write\"\n", q);
        fprintf(target_file, "PUSH R%d\n", q);
        fprintf(target_file, "MOV R%d, -2\n", q);
        fprintf(target_file, "PUSH R%d\n", q);
        fprintf(target_file, "PUSH R%d\n", p);
        fprintf(target_file, "PUSH R%d\n", q);
        fprintf(target_file, "PUSH R%d\n", q);
        fprintf(target_file, "CALL 0\n");
        fprintf(target_file, "POP R%d\n", q);
        fprintf(target_file, "POP R%d\n", q);
        fprintf(target_file, "POP R%d\n", q);
        fprintf(target_file, "POP R%d\n", q);
        fprintf(target_file, "POP R%d\n", q);
        freeReg();
        freeReg();
        return -1;
    }

    else if (t->nodetype == CONNECTOR)
    {
        codeGen(t->left, LST, target_file);
        codeGen(t->right, LST, target_file);
        return -1;
    }

    else if (t->nodetype == OPERATOR)
    {
        if (t->type == BOOLEAN)
        {
            p = codeGen(t->left, LST, target_file);
            q = codeGen(t->right, LST, target_file);
            if (strcmp(t->s, ">") == 0)
            {
                fprintf(target_file, "GT R%d, R%d\n", p, q);
            }
            else if (strcmp(t->s, "<") == 0)
            {
                fprintf(target_file, "LT R%d, R%d\n", p, q);
            }
            else if (strcmp(t->s, ">=") == 0)
            {
                fprintf(target_file, "GE R%d, R%d\n", p, q);
            }
            else if (strcmp(t->s, "<=") == 0)
            {
                fprintf(target_file, "LE R%d, R%d\n", p, q);
            }
            else if (strcmp(t->s, "==") == 0)
            {
                fprintf(target_file, "EQ R%d, R%d\n", p, q);
            }
            else if (strcmp(t->s, "!=") == 0)
            {
                fprintf(target_file, "NE R%d, R%d\n", p, q);
            }
            else if (strcmp(t->s, "&&") == 0)
            {
                fprintf(target_file, "MUL R%d, R%d\n", p, q);
            }
            else if (strcmp(t->s, "||") == 0)
            {
                fprintf(target_file, "ADD R%d, R%d\n", p, q);
            }
            freeReg();
            return p;
        }

        if (t->s[0] == '=')
        {
            p = codeGen(t->right, LST, target_file);

            if (t->left->nodetype == VARIABLE)
            {
                addrReg = getAddr(t->left, LST, target_file);
                fprintf(target_file, "MOV [R%d], R%d\n", addrReg, p);
                freeReg();
            }
            else if (t->left->nodetype == ARRAY)
            {
                addrReg = getArrayAddr(t->left, LST, target_file);
                fprintf(target_file, "MOV [R%d], R%d\n", addrReg, p);
                freeReg();
            }
            freeReg();
            return -1;
        }

        else
        {
            p = codeGen(t->left, LST, target_file);
            q = codeGen(t->right, LST, target_file);
            switch (t->s[0])
            {
            case '+':
                fprintf(target_file, "ADD R%d, R%d\n", p, q);
                freeReg();
                return p;
            case '-':
                fprintf(target_file, "SUB R%d, R%d\n", p, q);
                freeReg();
                return p;
            case '*':
                fprintf(target_file, "MUL R%d, R%d\n", p, q);
                freeReg();
                return p;
            case '/':
                fprintf(target_file, "DIV R%d, R%d\n", p, q);
                freeReg();
                return p;
            case '%':
                fprintf(target_file, "MOD R%d, R%d\n", p, q);
                freeReg();
                return p;
            }
        }
    }

    else if (t->nodetype == WHILE)
    {
        int u, v;
        numWhile++;
        u = getLabel();
        v = getLabel();
        labelIn[numWhile] = u;
        labelOut[numWhile] = v;
        fprintf(target_file, "L%d:\n", u);
        p = codeGen(t->left, LST, target_file);
        fprintf(target_file, "JZ R%d, L%d\n", p, v);
        freeReg();
        p = codeGen(t->right, LST, target_file);
        fprintf(target_file, "JMP L%d\n", u);
        fprintf(target_file, "L%d:\n", v);
        numWhile--;
        return -1;
    }

    else if (t->nodetype == IF)
    {
        int r, s;
        s = getLabel();
        p = codeGen(t->left, LST, target_file);
        fprintf(target_file, "JZ R%d, L%d\n", p, s);
        freeReg();
        p = codeGen(t->mid, LST, target_file);
        if (t->right != NULL)
        {
            r = getLabel();
            fprintf(target_file, "JMP L%d\n", r);
            fprintf(target_file, "L%d:\n", s);
            p = codeGen(t->right, LST, target_file);
            fprintf(target_file, "L%d:\n", r);
        }
        else
        {
            fprintf(target_file, "L%d:\n", s);
        }
        return -1;
    }

    else if (t->nodetype == BREAK)
    {
        if (numWhile)
            fprintf(target_file, "JMP L%d\n", labelOut[numWhile]);
        return -1;
    }

    else if (t->nodetype == CONTINUE)
    {
        if (numWhile)
            fprintf(target_file, "JMP L%d\n", labelIn[numWhile]);
        return -1;
    }

    else if (t->nodetype == ARRAY)
    {
        p = getReg();
        addrReg = getArrayAddr(t, LST, target_file);
        fprintf(target_file, "MOV R%d, [R%d]\n", p, addrReg);
        freeReg();
        return p;
    }

    else if (t->nodetype == FUNCTION)
    {
        struct GST_Node *curr = GSTLookup(t->s);
        fprintf(target_file, "F%d:\n", curr->flabel);
        fprintf(target_file, "PUSH BP\n");
        fprintf(target_file, "MOV BP, SP\n");
        fprintf(target_file, "ADD SP, %d\n", LST->size - ParamGetCount(ArgToParam(t->right)));
        if (t->left)
            p = codeGen(t->left, LST, target_file);
        return -1;
    }

    else if (t->nodetype == FUNCTIONCALL)
    {
        // Push all registers in use
        int regs = free_reg;
        for (int i = 0; i <= regs; i++)
        {
            fprintf(target_file, "PUSH R%d\n", i);
            freeReg();
        }
        // Push arguments
        int numArgs = 0;
        if (t->arg_list)
            numArgs = pushArgs(t->arg_list, numArgs, LST, target_file);
        // Push space for return value
        p = getReg();
        fprintf(target_file, "PUSH R%d\n", p);
        freeReg();
        // Call function
        int f = getFLabel(t->name);
        fprintf(target_file, "CALL F%d\n", f);
        // Get back all registers in use
        for (int i = regs; i >= 0; i--)
            getReg();
        // Pop return value
        p = getReg();
        fprintf(target_file, "POP R%d\n", p);
        // Pop arguments
        q = getReg();
        for (int i = 0; i < numArgs; i++)
            fprintf(target_file, "POP R%d\n", q);
        freeReg();
        // Pop registers in use
        for (int i = regs; i >= 0; i--)
        {
            fprintf(target_file, "POP R%d\n", i);
        }
        return p;
    }

    else if (t->nodetype == RET)
    {
        p = getReg();
        q = codeGen(t->left, LST, target_file);
        fprintf(target_file, "MOV R%d, BP\n", p);
        fprintf(target_file, "SUB R%d, 2\n", p);
        fprintf(target_file, "MOV [R%d], R%d\n", p, q);
        freeReg();
        freeReg();
        fprintf(target_file, "MOV SP, BP\n");
        fprintf(target_file, "POP BP\n");
        fprintf(target_file, "RET\n");
        return -1;
    }
}
//---------------------------Code Generation-----------------------------

//---------------------------Auxiliary Functions-----------------------------
int getAddr(struct AST_Node *t, struct LSTable *LST, FILE *target_file)
{
    char *name = t->name;
    struct LST_Node *l = LSTLookup(LST, name);
    if (l != NULL)
    {
        int p = getReg();
        fprintf(target_file, "MOV R%d, BP\n", p);
        fprintf(target_file, "ADD R%d, %d\n", p, l->binding);
        return p;
    }
    else
    {
        struct GST_Node *g = GSTLookup(name);
        int p = getReg();
        fprintf(target_file, "MOV R%d, %d\n", p, g->binding);
        return p;
    }
}

int getArrayAddr(struct AST_Node *t, struct LSTable *LST, FILE *target_file)
{
    struct GST_Node *g = GSTLookup(t->name);
    int p = getReg();
    fprintf(target_file, "MOV R%d, %d\n", p, g->binding);
    int q = codeGen(t->left, LST, target_file);
    fprintf(target_file, "ADD R%d, R%d\n", p, q);
    freeReg();
    return p;
}

int pushArgs(struct AST_Node *root, int numArgs, struct LSTable *LST, FILE *target_file)
{
    if (root)
    {
        numArgs++;
        numArgs = pushArgs(root->next_arg, numArgs, LST, target_file);
        int p = codeGen(root, LST, target_file);
        fprintf(target_file, "PUSH R%d\n", p);
        freeReg();
    }
    return numArgs;
}

struct LSTable *LSTParamInstall(struct LSTable *table, struct ParamNode *Phead)
{
    struct ParamNode *curr = Phead;
    int i = -3;
    while (curr)
    {
        table = LSTInstall(table, curr->name, curr->type);
        table->tail->binding = i;
        i--;
        curr = curr->next;
    }
    return table;
}

void generateHeader(FILE *target_file)
{
    fprintf(target_file, "0\n2056\n0\n0\n0\n0\n0\n0\n");
    fprintf(target_file, "MOV SP, %d\n", SP);
    fprintf(target_file, "MOV BP, 4096\n");
    fprintf(target_file, "PUSH R0\n");
    fprintf(target_file, "CALL F0\n");
    fprintf(target_file, "INT 10\n");
}
//---------------------------Auxiliary Functions-----------------------------
