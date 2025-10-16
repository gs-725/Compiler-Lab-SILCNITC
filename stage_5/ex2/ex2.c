#include "ex2.h"

struct GST_Node *Ghead = NULL;
struct TST_Node *Thead = NULL;

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
    exit(1);
}

int getLabel()
{
    return label++;
}

//---------------------------Parameter Node Functions-----------------------------

struct ParamNode *ParamInstall(struct ParamNode *Phead, char *name, Type type, Nodetype typeofvar)
{
    struct ParamNode *new_node = (struct ParamNode *)malloc(sizeof(struct ParamNode));
    new_node->name = strdup(name);
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
        free(temp->name);
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
        if (temp->type == INTEGER)
            printf("int\t");
        else if (temp->type == STRING)
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

//---------------------------Tuple Symbol Table (TST) Functions-----------------------------

struct TST_Node *TSTLookup(char *name)
{
    struct TST_Node *temp = Thead;
    while (temp != NULL)
    {
        if (strcmp(temp->name, name) == 0)
            return temp;
        temp = temp->next;
    }
    return NULL;
}

struct TST_Node *TSTInstall(char *name, struct ParamNode *Fhead)
{
    struct TST_Node *new_node = TSTLookup(name);
    if (new_node != NULL)
    {
        return new_node; 
    }
    
    new_node = (struct TST_Node *)malloc(sizeof(struct TST_Node));
    new_node->name = strdup(name);
    new_node->Fhead = ParamCopy(Fhead);
    new_node->size = ParamGetCount(new_node->Fhead);
    new_node->next = NULL;
    
    if (Thead == NULL)
    {
        Thead = new_node;
    }
    else
    {
        struct TST_Node *temp = Thead;
        while (temp->next != NULL)
            temp = temp->next;
        temp->next = new_node;
    }
    return new_node;
}

struct ParamNode *TSTGetField(struct TST_Node *tst, char *field_name)
{
    if (tst == NULL) return NULL;
    struct ParamNode *temp = tst->Fhead;
    while (temp != NULL)
    {
        if (strcmp(temp->name, field_name) == 0)
            return temp;
        temp = temp->next;
    }
    return NULL;
}

int TSTGetFieldOffset(struct TST_Node *tst, char *field_name)
{
    if (tst == NULL) return -1;
    struct ParamNode *temp = tst->Fhead;
    int offset = 0;
    while (temp != NULL)
    {
        if (strcmp(temp->name, field_name) == 0)
            return offset;
        offset++;
        temp = temp->next;
    }
    return -1; 
}

int TSTGetTupleSize(char *name) {
    struct TST_Node *tst = TSTLookup(name);
    if (!tst) return 0;
    return tst->size;
}

void TSTPrint() {
    struct TST_Node *temp = Thead;
    printf("TST:\n");
    while (temp) {
        printf("Tuple Name: %s, Size: %d\n", temp->name, temp->size);
        printf("Fields:\n");
        struct ParamNode *field = temp->Fhead;
        int offset = 0;
        while (field) {
            printf("  Offset %d: %s (Type: ", offset, field->name);
            if (field->type == INTEGER) printf("int");
            else if (field->type == STRING) printf("str");
            else if (field->type == BOOLEAN) printf("bool");
            else if (field->type == TUPLE) printf("tuple");
            else printf("other");
            printf(")\n");
            field = field->next;
            offset++;
        }
        temp = temp->next;
    }
}

//---------------------------Local Symbol Table (LST) Functions-----------------------------

struct LST_Node *LSTInitNode(char *name, Type type, int binding, struct TST_Node *TSType)
{
    struct LST_Node *new_node = (struct LST_Node *)malloc(sizeof(struct LST_Node));
    new_node->name = strdup(name);
    new_node->type = type;
    new_node->binding = binding;
    new_node->next = NULL;
    new_node->TSType = TSType; 
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

struct LSTable *LSTInstall(struct LSTable *table, char *name, Type type, struct TST_Node *TSType)
{
    struct LST_Node *new_node = LSTLookup(table, name);
    if (new_node != NULL)
    {
        printf("Error: Variable \"%s\" redeclared (LST)\n", name);
        exit(1);
    }
    
    int size = 1; 
    
    if (type == TUPLE && TSType != NULL) {
        size = TSType->size;
    }

    int binding_start = table->size + 1;
    table->size += size; 

    if (table->head == NULL)
    {
        table->head = table->tail = LSTInitNode(name, type, binding_start, TSType);
    }
    else
    {
        table->tail->next = LSTInitNode(name, type, binding_start, TSType);
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
        free(temp2->name);
        free(temp2);
    }
    free(table);
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
            new_table->head = new_table->tail = LSTInitNode(temp->name, temp->type, temp->binding, temp->TSType);
        }
        else
        {
            new_table->tail->next = LSTInitNode(temp->name, temp->type, temp->binding, temp->TSType);
            new_table->tail = new_table->tail->next;
        }
        temp = temp->next;
    }
    new_table->size = table->size;
    return new_table;
}

void LSTChangeType(struct LSTable *table, struct AST_Node *root, Type base_type, struct TST_Node *TSType)
{
    if (root == NULL) return;

    struct LST_Node *temp = LSTLookup(table, root->name);
    if (temp == NULL) {
        printf("Error: Variable \"%s\" undeclared (LST) during type setting.\n", root->name);
        exit(1);
    }

    if (base_type == TUPLE) {
        int old_size = (temp->type == TUPLE && temp->TSType) ? temp->TSType->size : 1;
        int new_size = TSType->size;
        
        if (old_size != new_size) {
            table->size = table->size - old_size + new_size;
        }

        temp->type = TUPLE;
        temp->TSType = TSType;
        root->type = TUPLE;
        root->TSTentry = TSType;
    } 
    else if (temp->type == PVOID) {
        if (base_type == INTEGER) temp->type = POINTER_TO_INTEGER;
        else if (base_type == STRING) temp->type = POINTER_TO_STRING;
        else {
             printf("Error: Invalid base type for local pointer variable \"%s\".\n", root->name);
             exit(1);
        }
    } else {
        temp->type = base_type;
    }
    
    root->type = temp->type;
}

void LSTPrint(struct LSTable *table)
{
    struct LST_Node *temp = table->head;
    printf("LST:\n");
    while (temp)
    {
        printf("%s\t", temp->name);
        if (temp->type == INTEGER) printf("int\t");
        else if (temp->type == STRING) printf("str\t");
        else if (temp->type == POINTER_TO_INTEGER) printf("int*\t");
        else if (temp->type == POINTER_TO_STRING) printf("str*\t");
        else if (temp->type == TUPLE) printf("tuple(%s)\t", temp->TSType->name);
        
        printf("binding:%d\n", temp->binding);
        temp = temp->next;
    }
}

//---------------------------Global Symbol Table (GST) Functions-----------------------------

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

struct GST_Node *GSTInstall(char *name, Type type, int size, Nodetype typeofvar, struct ParamNode *Phead, struct TST_Node *TSType)
{
    struct GST_Node *new_node = GSTLookup(name);
    if (new_node != NULL)
    {
        printf("Variable \"%s\" already declared\n", name);
        exit(1);
    }
    new_node = (struct GST_Node *)malloc(sizeof(struct GST_Node));
    new_node->name = strdup(name);
    new_node->type = type;
    new_node->typeofvar = typeofvar;
    new_node->Phead = Phead;
    new_node->next = NULL;
    new_node->TSType = TSType; 

    if (type == TUPLE && TSType != NULL) {
        new_node->size = TSType->size;
    } else {
        new_node->size = size;
    }
    
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
        new_node->binding = allocate(new_node->size); 
        new_node->flabel = -1;
    }
    
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
    return new_node;
}

void GSTChangeType(struct AST_Node *root, Type type, struct TST_Node *TSType)
{
    if (root != NULL) {
        if (root->nodetype == VARIABLE) {
            struct GST_Node *temp = GSTLookup(root->name);
            if (temp != NULL) {
                if (type == TUPLE) {
                    int old_size = temp->size;
                    int new_size = TSType->size;
                    
                    SP = SP - old_size + new_size;
                    
                    temp->type = TUPLE;
                    temp->TSType = TSType;
                    temp->size = new_size;
                    root->type = TUPLE;
                    root->TSTentry = TSType;
                }
                else if (temp->typeofvar == POINTER) {
                    if (type == INTEGER) temp->type = POINTER_TO_INTEGER;
                    else if (type == STRING) temp->type = POINTER_TO_STRING;
                    else {
                         printf("Error: Invalid base type for global pointer variable \"%s\".\n", root->name);
                         exit(1);
                    }
                } else {
                    temp->type = type;
                }
                root->type = temp->type;
            }
        }
        GSTChangeType(root->left, type, TSType);
        GSTChangeType(root->right, type, TSType);
    }
}

void GSTPrint()
{
    char *type;
    char *typeofvar;
    struct GST_Node *temp = Ghead;
    printf("%-12s\t%-15s\t%-5s\t%-10s\t%-7s\n", "Name", "Type", "Size", "TypeofVar", "Binding");
    while (temp != NULL)
    {
        if (temp->type == INTEGER) type = strdup("int");
        else if (temp->type == STRING) type = strdup("str");
        else if (temp->type == POINTER_TO_INTEGER) type = strdup("int*");
        else if (temp->type == POINTER_TO_STRING) type = strdup("str*");
        else if (temp->type == TUPLE) {
            char t_name[20];
            snprintf(t_name, sizeof(t_name), "tuple(%s)", temp->TSType->name);
            type = strdup(t_name);
        }

        if (temp->typeofvar == ARRAY) typeofvar = strdup("array");
        else if (temp->typeofvar == VARIABLE) typeofvar = strdup("variable");
        else if (temp->typeofvar == FUNCTION) typeofvar = strdup("function");

        printf("%-12s\t%-15s\t%-5d\t%-10s\t%-7d\n", temp->name, type, temp->size, typeofvar, temp->binding);
        free(type);
        free(typeofvar);
        temp = temp->next;
    }
}

//---------------------------AST Node Constructors-----------------------------

struct AST_Node *makeVariableLeafNode(char *name, char *s)
{
    struct AST_Node *new_node = (struct AST_Node *)malloc(sizeof(struct AST_Node));
    new_node->s = strdup(s);
    new_node->nodetype = VARIABLE;
    new_node->name = strdup(name);
    new_node->GSTentry = GSTLookup(name);
    if (new_node->GSTentry)
        new_node->type = new_node->GSTentry->type;
    else
        new_node->type = VOID;
    new_node->TSTentry = NULL;
    new_node->left = NULL;
    new_node->right = NULL;
    new_node->mid = NULL;
    new_node->next_arg = NULL;
    new_node->arg_list = NULL;
    return new_node;
}

struct AST_Node *makeConstantLeafNode(Type type, int val, char *s)
{
    struct AST_Node *new_node = (struct AST_Node *)malloc(sizeof(struct AST_Node));
    new_node->s = strdup(s);
    new_node->nodetype = CONSTANT;
    new_node->type = type;
    new_node->val = val;
    new_node->GSTentry = NULL;
    new_node->TSTentry = NULL;
    new_node->left = NULL;
    new_node->right = NULL;
    new_node->mid = NULL;
    new_node->name = NULL;
    new_node->next_arg = NULL;
    new_node->arg_list = NULL;
    return new_node;
}

struct AST_Node *makeArrayLeafNode(char *name, struct AST_Node *l, char *s)
{
    struct AST_Node *new_node = (struct AST_Node *)malloc(sizeof(struct AST_Node));
    new_node->s = strdup(s);
    new_node->nodetype = ARRAY;
    new_node->name = strdup(name);
    new_node->GSTentry = GSTLookup(name);
    new_node->type = new_node->GSTentry->type;
    new_node->TSTentry = NULL;
    new_node->left = l;
    new_node->right = NULL;
    new_node->mid = NULL;
    new_node->next_arg = NULL;
    new_node->arg_list = NULL;
    return new_node;
}

struct AST_Node *makeTupleFieldAccessNode(struct AST_Node *tuple_var_node, char *field_name, char *s)
{
    struct AST_Node *new_node = (struct AST_Node *)malloc(sizeof(struct AST_Node));
    new_node->s = strdup(s);
    new_node->nodetype = TUPLE_FIELD_ACCESS;
    new_node->name = strdup(field_name); 
    new_node->left = tuple_var_node;     
    new_node->mid = NULL;
    new_node->right = NULL;
    new_node->GSTentry = NULL;
    new_node->next_arg = NULL;
    new_node->arg_list = NULL;

    if (tuple_var_node->type != TUPLE) {
        printf("Line %d: Variable '%s' is not a tuple type.\n", yylineno, tuple_var_node->name);
        exit(1);
    }

    struct TST_Node *tst_def = tuple_var_node->GSTentry ? tuple_var_node->GSTentry->TSType : tuple_var_node->TSTentry;
    
    if (tst_def == NULL) {
        printf("Line %d: Could not resolve tuple type definition for '%s'.\n", yylineno, tuple_var_node->name);
        exit(1);
    }
    
    struct ParamNode *field_def = TSTGetField(tst_def, field_name);
    if (field_def == NULL) {
        printf("Line %d: Tuple type '%s' has no field named '%s'.\n", yylineno, tst_def->name, field_name);
        exit(1);
    }

    new_node->type = field_def->type;
    new_node->TSTentry = tst_def; 

    return new_node;
}

struct AST_Node *makeNode(Nodetype node_type, Type type, struct AST_Node *l, struct AST_Node *m, struct AST_Node *r, struct GST_Node *gst, char *s)
{
    struct AST_Node *new_node = (struct AST_Node *)malloc(sizeof(struct AST_Node));
    
    if (node_type == OPERATOR) {
        if (strcmp(s, "=") == 0) {
            if (l->nodetype != VARIABLE && l->nodetype != ARRAY && l->nodetype != POINTER && l->nodetype != TUPLE_FIELD_ACCESS) {
                printf("Error: Left side of assignment must be an assignable location.\n");
                exit(1);
            }
            
            if (l->type == TUPLE) {
                 if (r->type != TUPLE) {
                     printf("Error: Type mismatch in tuple assignment. Must assign tuple to tuple.\n");
                     exit(1);
                 }
            } else {
                int assignment_allowed = 0;
                if (r->nodetype == VARIABLE && r->GSTentry && r->GSTentry->typeofvar == ARRAY) {
                    if ((l->type == POINTER_TO_INTEGER && r->GSTentry->type == INTEGER) || 
                        (l->type == POINTER_TO_STRING && r->GSTentry->type == STRING)) {
                        assignment_allowed = 1;
                    }
                }
                else if (r->nodetype == ADDRESS) {
                    if (l->type == r->type) {
                        assignment_allowed = 1;
                    }
                }
                else if ((l->type == POINTER_TO_INTEGER && r->type == POINTER_TO_INTEGER) ||
                        (l->type == POINTER_TO_STRING && r->type == POINTER_TO_STRING)) {
                    assignment_allowed = 1;
                }
                else {
                    if (l->type != r->type) { 
                        Type l_base_type = l->type;
                        Type r_base_type = r->type;

                        if (l_base_type == POINTER_TO_INTEGER) l_base_type = INTEGER;
                        else if (l_base_type == POINTER_TO_STRING) l_base_type = STRING;
                        
                        if (r_base_type == POINTER_TO_INTEGER) r_base_type = INTEGER;
                        else if (r_base_type == POINTER_TO_STRING) r_base_type = STRING;
                        
                        if (l_base_type != r_base_type) {
                             printf("Error: Type mismatch in assignment. Expected type %d, got %d.\n", l->type, r->type);
                             exit(1);
                        }
                        assignment_allowed = 1; 
                    } else {
                        assignment_allowed = 1; 
                    }
                }
                
                if (!assignment_allowed && l->type != TUPLE) {
                    printf("Error: Invalid assignment type mismatch.\n");
                    exit(1);
                }
            }
        }
        else if (l->type >= POINTER_TO_INTEGER || r->type >= POINTER_TO_INTEGER) {
            if (l->type != POINTER_TO_INTEGER && r->type != POINTER_TO_INTEGER) {
                printf("Error: Invalid operator '%s' for non-integer pointer types.\n", s);
                exit(1);
            }
            
            if (strcmp(s, "+") == 0 || strcmp(s, "-") == 0) {
                if ((l->type == POINTER_TO_INTEGER && r->type == INTEGER) || 
                    (l->type == INTEGER && r->type == POINTER_TO_INTEGER) ||
                    (l->type == POINTER_TO_INTEGER && r->type == POINTER_TO_INTEGER && strcmp(s, "-") == 0)) {
                    
                    if (l->type == POINTER_TO_INTEGER && r->type == POINTER_TO_INTEGER) {
                        type = INTEGER;
                    } else {
                        type = POINTER_TO_INTEGER;
                    }
                } else {
                        printf("Error: Invalid pointer arithmetic. Only (pointer +/- integer) or (pointer - pointer) is allowed.\n");
                        exit(1);
                }
            } else {
                printf("Error: Invalid operator '%s' for pointer types. Only '+', '-' allowed.\n", s);
                exit(1);
            }
        }
        else if (type == INTEGER) {
            if (l->type != INTEGER || r->type != INTEGER) {
                printf("Error: Type mismatch. Arithmetic operator requires integer operands.\n");
                exit(1);
            }
        }
        else if (type == BOOLEAN) {
            if (l->type != r->type) { 
                printf("Error: Type mismatch. Relational operator requires compatible operands.\n");
                exit(1);
            }
        }
    }

    if (node_type == WHILE || node_type == IF || node_type == REPEAT || node_type == DOWHILE) {
        struct AST_Node *condition = (node_type == IF || node_type == WHILE) ? l : r; 

        if (condition->type != BOOLEAN) {
            printf("Error: Type mismatch. Condition for loop/IF must be BOOLEAN.\n");
            exit(1);
        }
    }

    new_node->s = strdup(s);
    new_node->nodetype = node_type;
    new_node->type = type;
    new_node->left = l;
    new_node->mid = m;
    new_node->right = r;
    new_node->GSTentry = gst;
    new_node->TSTentry = NULL;
    new_node->name = NULL;
    new_node->next_arg = NULL;
    new_node->arg_list = NULL;
    return new_node;
}

struct AST_Node *makePointerNode(Nodetype node_type, struct AST_Node *l, char *s)
{
    struct AST_Node *new_node = (struct AST_Node *)malloc(sizeof(struct AST_Node));
    new_node->s = strdup(s);
    new_node->nodetype = node_type;
    new_node->left = l;
    new_node->mid = NULL;
    new_node->right = NULL;
    new_node->GSTentry = NULL;
    new_node->TSTentry = NULL;
    new_node->name = NULL;
    new_node->next_arg = NULL;
    new_node->arg_list = NULL;

    if (node_type == ADDRESS) { 
        if (l->nodetype != VARIABLE && l->nodetype != ARRAY && l->nodetype != TUPLE_FIELD_ACCESS) {
            printf("Error: Address-of operator '&' can only be applied to variables, arrays, or tuple fields.\n");
            exit(1);
        }
        
        switch (l->type) {
            case INTEGER:
            case TUPLE:
                new_node->type = POINTER_TO_INTEGER;
                break;
            case STRING:
                new_node->type = POINTER_TO_STRING;
                break;
            case POINTER_TO_INTEGER: 
            case POINTER_TO_STRING:
                new_node->type = POINTER_TO_INTEGER;
                break;
            default:
                printf("Error: Invalid type for address-of operator (Type %d).\n", l->type);
                exit(1);
        }
    } else if (node_type == POINTER) { 
        if (l->type == POINTER_TO_INTEGER) new_node->type = INTEGER;
        else if (l->type == POINTER_TO_STRING) new_node->type = STRING;
        else {
            printf("Error: Dereference operator '*' can only be applied to a pointer variable (Got Type %d).\n", l->type);
            exit(1);
        }
    }
    return new_node;
}

struct AST_Node *ASTChangeTypeGST(struct AST_Node *root, Type type, struct TST_Node *TSType)
{
    if (root != NULL)
    {
        root->left = ASTChangeTypeGST(root->left, type, TSType);
        root->right = ASTChangeTypeGST(root->right, type, TSType);
        if (root->nodetype == VARIABLE)
        {
            GSTChangeType(root, type, TSType);
        }
        free(root);
    }
    return NULL;
}

struct AST_Node *ASTChangeTypeLST(struct LSTable *table, struct AST_Node *root, Type type, struct TST_Node *TSType)
{
    if (root != NULL)
    {
        root->left = ASTChangeTypeLST(table, root->left, type, TSType);
        root->right = ASTChangeTypeLST(table, root->right, type, TSType);
        if (root->nodetype == VARIABLE || root->nodetype == POINTER)
        {
            LSTChangeType(table, root, type, TSType);
        }
        free(root);
    }
    return NULL;
}

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

int numWhile = 0;
int labelIn[100], labelOut[100];

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
        //printf("Local variable '%s' found at BP + %d Reg no is %d\n", name, l->binding, p);
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

int getFieldAddr(struct AST_Node *t, struct LSTable *LST, FILE *target_file)
{
    int addrReg = getAddr(t->left, LST, target_file);
    struct TST_Node *tst_def = t->TSTentry;
    int offset = TSTGetFieldOffset(tst_def, t->name);
    if (offset < 0) {
        printf("CodeGen Error: Field '%s' not found in tuple type.\n", t->name);
        exit(1);
    }

    if (offset > 0) {
        int offsetReg = getReg();
        fprintf(target_file, "MOV R%d, %d\n", offsetReg, offset);
        fprintf(target_file, "ADD R%d, R%d\n", addrReg, offsetReg);
        freeReg();
    }

    return addrReg;
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
        struct LST_Node *new_node = LSTInitNode(curr->name, curr->type, i, NULL);
        
        if (table->head == NULL) {
             table->head = table->tail = new_node;
        } else {
             table->tail->next = new_node;
             table->tail = new_node;
        }
        
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

//---------------------------Code Generation-----------------------------

int codeGen(struct AST_Node *t, struct LSTable *LST, FILE *target_file)
{
    int p, q, r, s, addrReg;

    if (t == NULL) return -1;

    if (t->nodetype == CONSTANT)
    {
        p = getReg();
        if (t->type == INTEGER)
        {
            fprintf(target_file, "MOV R%d, %d\n", p, t->val);
        }
        else if (t->type == STRING)
        {
            fprintf(target_file, "MOV R%d, %s\n", p, t->s);
        }
        return p;
    }

    else if (t->nodetype == ADDRESS) {
        if (t->left->nodetype == TUPLE_FIELD_ACCESS) {
             return getFieldAddr(t->left, LST, target_file);
        }
        int addr = getAddr(t->left, LST, target_file);
        return addr;
    }

    else if (t->nodetype == VARIABLE)
    {
        p = getReg();
        addrReg = getAddr(t, LST, target_file);

        struct GST_Node* g = GSTLookup(t->name);
        struct LST_Node* l = LSTLookup(LST, t->name);
        
        if ((g && (g->typeofvar == ARRAY || g->type == TUPLE)) || 
            (l && l->type == TUPLE)) {
             fprintf(target_file, "MOV R%d, R%d\n", p, addrReg);
        }
        else if ((g && g->typeofvar == POINTER) || (l && l->type >= POINTER_TO_INTEGER)) {
             fprintf(target_file, "MOV R%d, [R%d]\n", p, addrReg);
        }
        else {
            fprintf(target_file, "MOV R%d, [R%d]\n", p, addrReg);
        }

        freeReg();
        return p;
    }

    else if (t->nodetype == POINTER) {
        p = codeGen(t->left, LST, target_file);
        fprintf(target_file, "MOV R%d, [R%d]\n", p, p);
        return p;
    }

    else if (t->nodetype == TUPLE_FIELD_ACCESS)
    {
        p = getReg();
        addrReg = getFieldAddr(t, LST, target_file); 
        
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
        else if (t->left->nodetype == POINTER)
        {
            addrReg = codeGen(t->left->left, LST, target_file);
            fprintf(target_file, "MOV R%d, R%d\n", p, addrReg);
            freeReg();
        } 
        else if (t->left->nodetype == TUPLE_FIELD_ACCESS) 
        {
            addrReg = getFieldAddr(t->left, LST, target_file);
            fprintf(target_file, "MOV R%d, R%d\n", p, addrReg);
            freeReg();
        } else {
             printf("Error: Invalid L-Value in READ statement.\n");
             exit(1);
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
            if (t->type == TUPLE) {
                struct TST_Node *tst_def = t->left->GSTentry ? t->left->GSTentry->TSType : t->left->TSTentry;
                int size = tst_def->size;
                // printf("name of tuple: %s\n", t->left->name);
                // printf("/* Tuple Assignment of size %d */\n", size);

                int l_addr = getAddr(t->left, LST, target_file); 
                int r_addr = getAddr(t->right, LST, target_file); 

                fprintf(target_file, "MOV R16, R%d\n", l_addr);
                fprintf(target_file, "MOV R17, R%d\n", r_addr);

                freeReg();
                freeReg();

                fprintf(target_file, "MOV R18, 0\n");
                int loop_label = getLabel();
                int end_label = getLabel();
                
                fprintf(target_file, "L%d:\n", loop_label);
                
                fprintf(target_file, "MOV R15, %d\n", size);
                fprintf(target_file, "SUB R15, R18\n");
                fprintf(target_file, "JZ R15, L%d\n", end_label);
                
                fprintf(target_file, "MOV R15, R17\n");
                fprintf(target_file, "ADD R15, R18\n");
                fprintf(target_file, "MOV R19, [R15]\n");
                
                fprintf(target_file, "MOV R15, R16\n");
                fprintf(target_file, "ADD R15, R18\n");
                fprintf(target_file, "MOV [R15], R19\n");
                
                fprintf(target_file, "ADD R18, 1\n");
                fprintf(target_file, "JMP L%d\n", loop_label);
                fprintf(target_file, "L%d:\n", end_label);

                return -1;
            }
            
            p = codeGen(t->right, LST, target_file); 
            
            if (t->left->nodetype == POINTER) { 
                addrReg = codeGen(t->left->left, LST, target_file);
                fprintf(target_file, "MOV [R%d], R%d\n", addrReg, p);
                freeReg();
            } else if (t->left->nodetype == TUPLE_FIELD_ACCESS) { 
                addrReg = getFieldAddr(t->left, LST, target_file);
                fprintf(target_file, "MOV [R%d], R%d\n", addrReg, p);
                freeReg();
            } else if (t->left->nodetype == VARIABLE) {
                addrReg = getAddr(t->left, LST, target_file);
                fprintf(target_file, "MOV [R%d], R%d\n", addrReg, p);
                freeReg();
            } else if (t->left->nodetype == ARRAY) {
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
            
            if (t->left->type == POINTER_TO_INTEGER || t->right->type == POINTER_TO_INTEGER) {
                if (strcmp(t->s, "+") == 0) {
                    fprintf(target_file, "ADD R%d, R%d\n", p, q);
                } else if (strcmp(t->s, "-") == 0) {
                    fprintf(target_file, "SUB R%d, R%d\n", p, q);
                }
                freeReg();
                return p;
            }
            
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
        
        fprintf(target_file, "ADD SP, %d\n", LST->size); 
        
        if (t->left)
            p = codeGen(t->left, LST, target_file);
        
        if (strcmp(t->s, "main") != 0) {
            fprintf(target_file, "MOV SP, BP\n");
            fprintf(target_file, "POP BP\n");
            fprintf(target_file, "RET\n");
        }
        return -1;
    }

    else if (t->nodetype == FUNCTIONCALL)
    {
        int regs = free_reg;
        for (int i = 0; i <= regs; i++)
        {
            fprintf(target_file, "PUSH R%d\n", i);
            freeReg();
        }
        
        int numArgs = 0;
        if (t->arg_list)
            numArgs = pushArgs(t->arg_list, numArgs, LST, target_file);
            
        p = getReg();
        fprintf(target_file, "PUSH R%d\n", p);
        freeReg();
        
        int f = getFLabel(t->name);
        fprintf(target_file, "CALL F%d\n", f);
        
        for (int i = regs; i >= 0; i--)
            getReg();
            
        p = getReg();
        fprintf(target_file, "POP R%d\n", p);
        
        q = getReg();
        for (int i = 0; i < numArgs; i++)
            fprintf(target_file, "POP R%d\n", q);
        freeReg();
        
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

    return -1;
}
