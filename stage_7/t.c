#include "t.h"

struct GST_Node *Ghead = NULL;
struct TypeTable *Thead = NULL;
struct ClassTable *Chead = NULL;
struct ClassTable *Ctail = NULL;
struct Stack *Shead = NULL;

int inClass = 0;

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
int free_reg = 0;

int getReg()
{
    return free_reg++;
}

void freeReg()
{
    free_reg--;
}
//---------------------------Register Allocation-----------------------------

//------------------------Label Allocation----------------------------

int label = 0;
int fLabel = 1;
int mLabel = 1;

int getFLabel()
{
    return fLabel++;
}

int getLabel()
{
    return label++;
}

int getMLabel()
{
    return mLabel++;
}

//------------------------Label Allocation------------------------------

//---------------------------Type Table-----------------------------
struct TypeTable *TTInstall(char *name, int size, struct FieldList *fields)
{

    struct TypeTable *new_node = TTLookup(name);
    if (new_node != NULL)
    {
        printf("Error: User-defined type \"%s\" already defined\n", name);
        exit(1);
    }
    new_node = (struct TypeTable *)malloc(sizeof(struct TypeTable));
    new_node->name = strdup(name);
    new_node->size = size;
    new_node->fields = fields;
    new_node->next = NULL;
    if (Thead == NULL)
        return new_node;
    struct TypeTable *temp = Thead;
    while (temp->next != NULL)
        temp = temp->next;
    temp->next = new_node;
    return Thead;
}

void TTCreate()
{
    Thead = NULL;
    Thead = TTInstall("int", 1, NULL);
    Thead = TTInstall("str", 1, NULL);
    Thead = TTInstall("boolean", 1, NULL);
    Thead = TTInstall("void", 1, NULL);
}

struct TypeTable *TTLookup(char *name)
{
    struct TypeTable *temp = Thead;
    while (temp != NULL)
    {
        if (strcmp(temp->name, name) == 0)
            return temp;
        temp = temp->next;
    }
    return NULL;
}

struct FieldListNode *TTLookupField(struct TypeTable *type, char *name)
{
    if (type == NULL)
        return NULL;
    struct FieldListNode *field = type->fields->head;
    while (field != NULL)
    {
        if (strcmp(field->name, name) == 0)
            return field;
        field = field->next;
    }
    return NULL;
}

void TTPrint()
{
    struct TypeTable *temp = Thead;
    printf("Type Table:\n");
    printf("%-10s%-3s\n", "Name", "Size");
    while (temp != NULL)
    {
        printf("%-10s%-3d\n", temp->name, temp->size);
        if (temp->fields != NULL)
            FLPrint(temp->fields);
        temp = temp->next;
    }
    printf("\n");
}
//---------------------------Type Table-----------------------------

//---------------------------Field List-----------------------------
struct FieldList *FLInit()
{
    struct FieldList *field_list = (struct FieldList *)malloc(sizeof(struct FieldList));
    field_list->head = field_list->tail = NULL;
    field_list->size = 0;
    return field_list;
}

struct FieldListNode *FLInitNode(char *name, int fieldIndex, struct TypeTable *type, struct ClassTable *class)
{
    struct FieldListNode *new_node = (struct FieldListNode *)malloc(sizeof(struct FieldListNode));
    new_node->name = strdup(name);
    new_node->fieldIndex = fieldIndex;
    new_node->type = type;
    new_node->class = class;
    new_node->next = NULL;
    return new_node;
}

struct FieldList *FLInstall(struct FieldList *field_list, char *name, struct TypeTable *type, struct ClassTable *class)
{
    struct FieldListNode *temp = field_list->head;
    while (temp != NULL)
    {
        if (strcmp(temp->name, name) == 0)
        {
            printf("Error: Field \"%s\" already defined\n", name);
            exit(1);
        }
        temp = temp->next;
    }
    struct FieldListNode *new_node = FLInitNode(name, field_list->size, type, class);
    if (field_list->head == NULL)
    {
        field_list->head = field_list->tail = new_node;
    }
    else
    {
        field_list->tail->next = new_node;
        field_list->tail = new_node;
    }
    field_list->size++;
    return field_list;
}

void FLPrint(struct FieldList *field_list)
{
    if (field_list == NULL)
        return;
    printf("Fields:\n");
    printf("Index\tName\tType\tClass\n");
    struct FieldListNode *temp = field_list->head;
    while (temp != NULL)
    {
        printf("%d\t", temp->fieldIndex);
        printf("%s\t", temp->name);
        printf("%s\t", (temp->type == NULL) ? "NULL" : temp->type->name);
        printf("%s\n", (temp->class == NULL) ? "NULL" : temp->class->name);
        temp = temp->next;
    }
}
//---------------------------Field List-----------------------------

//---------------------------Class Table-----------------------------
struct ClassTable *CInstall(char *name, char *parent)
{
    if (TTLookup(name) != NULL)
    {
        printf("Error: Cannot define class with name of previously defined type\n");
        printf("Class name: \"%s\"\n", name);
        exit(1);
    }
    if (CLookup(name) != NULL)
    {
        printf("Error: Class \"%s\" already defined\n", name);
        exit(1);
    }
    struct ClassTable *new_node = (struct ClassTable *)malloc(sizeof(struct ClassTable));
    new_node->name = strdup(name);
    new_node->classIndex = (Ctail == NULL) ? 0 : Ctail->classIndex + 1;
    new_node->fieldCount = new_node->methodCount = 0;
    new_node->fields = NULL;
    new_node->methods = NULL;
    new_node->parent = NULL; // for stage 8
    new_node->next = NULL;
    if (Chead == NULL)
    {
        Chead = Ctail = new_node;
        return new_node;
    }
    else
    {
        Ctail->next = new_node;
        Ctail = new_node;
    }
    return new_node;
}

struct ClassTable *CLookup(char *name)
{
    struct ClassTable *temp = Chead;
    while (temp != NULL)
    {
        if (strcmp(temp->name, name) == 0)
            return temp;
        temp = temp->next;
    }
    return NULL;
}

void CInstallField(struct ClassTable *class, char *name, struct TypeTable *type)
{
    if (CLookupField(class, name) != NULL)
    {
        printf("Error: Field \"%s\" already defined in class \"%s\"\n", name, class->name);
        exit(1);
    }
    if (class->fields == NULL)
        class->fields = FLInit();
    struct ClassTable *fieldClass = CLookup(type->name);
    class->fields = FLInstall(class->fields, name, type, (fieldClass) ? fieldClass : NULL);
    class->fieldCount++;
}

void CInstallMethod(struct ClassTable *class, char *name, struct TypeTable *type, struct ParamNode *Phead)
{
    if (CLookupMethod(class, name) != NULL)
    {
        printf("Error: Method \"%s\" already defined in class \"%s\"\n", name, class->name);
        exit(1);
    }
    if (class->methods == NULL)
        class->methods = MLInit();
    class->methods = MLInstall(class->methods, name, type, Phead);
    class->methodCount++;
}

struct MethodListNode *CLookupMethod(struct ClassTable *class, char *name)
{
    if (class == NULL)
        return NULL;
    if (class->methods == NULL)
        return NULL;
    struct MethodListNode *temp = class->methods->head;
    while (temp != NULL)
    {
        if (strcmp(temp->name, name) == 0)
            return temp;
        temp = temp->next;
    }
    return NULL;
}

struct FieldListNode *CLookupField(struct ClassTable *class, char *name)
{
    if (class == NULL)
        return NULL;
    if (class->fields == NULL)
        return NULL;
    struct FieldListNode *temp = class->fields->head;
    while (temp != NULL)
    {
        if (strcmp(temp->name, name) == 0)
            return temp;
        temp = temp->next;
    }
    return NULL;
}

void CPrint(struct ClassTable *class)
{
    printf("\nClass Table:\n");
    printf("Index\tName\t\tFields\tMethods\tParent\n");
    struct ClassTable *temp = class;
    while (temp != NULL)
    {
        printf("%d\t", temp->classIndex);
        printf("%-16s", temp->name);
        printf("%d\t", temp->fieldCount);
        printf("%d\t", temp->methodCount);
        printf("%s\n", (temp->parent == NULL) ? "NULL" : temp->parent->name);
        temp = temp->next;
    }
    printf("\n");
}
//---------------------------Class Table-----------------------------

//---------------------------Method List-----------------------------
struct MethodList *MLInit()
{
    struct MethodList *method_list = (struct MethodList *)malloc(sizeof(struct MethodList));
    method_list->head = method_list->tail = NULL;
    method_list->size = 0;
    return method_list;
}

struct MethodListNode *MLInitNode(char *name, int methodIndex, int mlabel, struct TypeTable *type, struct ParamNode *Phead)
{
    struct MethodListNode *new_node = (struct MethodListNode *)malloc(sizeof(struct MethodListNode));
    new_node->name = strdup(name);
    new_node->methodIndex = methodIndex;
    new_node->mlabel = mlabel;
    new_node->type = type;
    new_node->Phead = Phead;
    new_node->next = NULL;
    return new_node;
}

struct MethodList *MLInstall(struct MethodList *method_list, char *name, struct TypeTable *type, struct ParamNode *Phead)
{
    if (type == NULL)
    {
        printf("Error: Type of method \"%s\" not defined\n", name);
        exit(1);
    }
    struct MethodListNode *new_node = MLInitNode(name, method_list->size, getMLabel(), type, Phead);
    if (method_list->head == NULL)
    {
        method_list->head = method_list->tail = new_node;
    }
    else
    {
        method_list->tail->next = new_node;
        method_list->tail = new_node;
    }
    method_list->size++;
    return method_list;
}

void MLPrint(struct MethodList *method_list)
{
    if (method_list == NULL)
        return;
    printf("\nMethod List:\n");
    printf("Method Index\tName\tType\tParameter List\n");
    struct MethodListNode *temp = method_list->head;
    while (temp != NULL)
    {
        printf("%d\t\t", temp->methodIndex);
        printf("%s\t", temp->name);
        printf("%s\t", temp->type->name);
        printf("%s\n", temp->Phead->name);
        temp = temp->next;
    }
    printf("\n");
}
//---------------------------Method List-----------------------------

//---------------------------Parameter List-----------------------------
struct ParamNode *ParamInstall(struct ParamNode *Phead, char *name, struct TypeTable *type)
{
    if (type == NULL)
    {
        printf("Error: Type of parameter \"%s\" not defined\n", name);
        exit(1);
    }
    struct ParamNode *new_node = (struct ParamNode *)malloc(sizeof(struct ParamNode));
    new_node->name = name;
    new_node->type = type;
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
        new_head = ParamInstall(new_head, temp->name, temp->type);
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
        printf("%s\t", temp->type->name);
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
        if (temp1->type != temp2->type)
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
struct LST_Node *LSTInitNode(char *name, struct TypeTable *type, struct ClassTable *class, int binding)
{
    struct LST_Node *new_node = (struct LST_Node *)malloc(sizeof(struct LST_Node));
    new_node->name = strdup(name);
    new_node->type = type;
    new_node->class = class;
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

struct LSTable *LSTInstall(struct LSTable *table, char *name, struct TypeTable *type, struct ClassTable *class)
{
    if (type == NULL && class == NULL)
    {
        printf("Error: Type or class of variable \"%s\" not defined\n", name);
        exit(1);
    }
    struct LST_Node *new_node = LSTLookup(table, name);
    if (new_node != NULL)
    {
        printf("Error: Variable \"%s\" redeclared (LST)\n", name);
        exit(1);
    }
    table->size++;
    new_node = LSTInitNode(name, type, class, table->size);
    if (table->head == NULL)
    {
        table->head = table->tail = new_node;
    }
    else
    {
        table->tail->next = new_node;
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
            new_table->head = new_table->tail = LSTInitNode(temp->name, temp->type, temp->class, temp->binding);
        }
        else
        {
            new_table->tail->next = LSTInitNode(temp->name, temp->type, temp->class, temp->binding);
            new_table->tail = new_table->tail->next;
        }
        temp = temp->next;
    }
    new_table->size = table->size;
    return new_table;
}

void LSTChangeType(struct LSTable *table, struct AST_Node *root, struct TypeTable *type)
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
    printf("Name\tType\tClass\tBinding\n");
    while (temp)
    {
        printf("%s\t", temp->name);
        printf("%s\t", (temp->type) ? temp->type->name : NULL);
        printf("%s\t", (temp->class) ? temp->class->name : NULL);
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

struct GST_Node *GSTInstall(char *name, struct TypeTable *type, int size, Nodetype typeofvar, struct ParamNode *Phead, struct ClassTable *class)
{
    struct GST_Node *new_node = GSTLookup(name);
    if (new_node != NULL)
    {
        printf("Variable \"%s\" already declared\n", name);
        exit(1);
    }
    if (type == NULL)
    {
        printf("Error: (GST) Invalid type for variable \"%s\"\n", name);
        exit(1);
    }
    new_node = (struct GST_Node *)malloc(sizeof(struct GST_Node));
    new_node->name = name;
    new_node->type = type;
    new_node->size = size;
    new_node->class = class;
    new_node->typeofvar = typeofvar;
    if (typeofvar == FUNCTION)
    {
        new_node->binding = -1;
        if (strcmp(name, "main") == 0)
            new_node->flabel = 0;
        else
            new_node->flabel = getFLabel();
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

void GSTChangeTypeClass(struct AST_Node *root, struct TypeTable *type, struct ClassTable *class)
{
    struct GST_Node *temp = GSTLookup(root->name);
    if (temp == NULL)
    {
        printf("Variable \"%s\" not declared\n", root->name);
        exit(1);
    }
    temp->type = type;
    temp->class = class;
}

void GSTPrint()
{
    char *typeofvar;
    struct GST_Node *temp = Ghead;
    printf("GST:\n");
    printf("%-12s%-8s%-12s%-5s%-10s%-7s\n", "Name", "Type", "Class", "Size", "TypeofVar", "Binding");
    while (temp != NULL)
    {
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
        printf("%-12s", temp->name);
        printf("%-8s", (temp->type) ? temp->type->name : "NULL");
        printf("%-12s", (temp->class) ? temp->class->name : "NULL");
        printf("%-5d", temp->size);
        printf("%-10s", typeofvar);
        printf("%-7d\n", temp->binding);
        temp = temp->next;
    }
    printf("\n");
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

struct AST_Node *makeConstantLeafNode(struct TypeTable *type, int val, char *s)
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

struct AST_Node *makeNode(Nodetype node_type, struct TypeTable *type, struct ClassTable *class, struct AST_Node *l, struct AST_Node *m, struct AST_Node *r, struct GST_Node *gst, char *s)
{
    struct AST_Node *new_node = (struct AST_Node *)malloc(sizeof(struct AST_Node));

    // Type checking
    if (node_type == OPERATOR && !((strcmp(s, "=") == 0) || (strcmp(s, "!=") == 0) || (strcmp(s, "==") == 0)))
    {
        if (!typeCheckInt(l) || !typeCheckInt(r))
        {
            printf("Error: Type mismatch. \"Operand is not integer\". Operator: \"%s\"\n", s);
            exit(1);
        }
    }
    else
    {
        if (node_type == OPERATOR && !(strcmp(s, "=") == 0))
        {
            if (!((typeCheckInt(l) || (!typeCheckInt(l) && !typeCheckBool(l))) && (r->nodetype == NULL__ || typeCheckInt(r) || (!typeCheckInt(r) && !typeCheckBool(r)))))
            {
                printf("Error: Type mismatch in \"%s\".\n", s);
                exit(1);
            }
        }
    }

    if (node_type == WHILE || node_type == IF)
    {
        if (!typeCheckBool(l))
        {
            printf("Error: Type mismatch. \"Guard is not boolean\".\n");
            exit(1);
        }
    }

    new_node->s = (char *)malloc(sizeof(char) * strlen(s));
    new_node->s = strdup(s);
    new_node->nodetype = node_type;
    new_node->type = type;
    new_node->class = class;
    new_node->left = l;
    new_node->mid = m;
    new_node->right = r;
    new_node->GSTentry = gst;
    new_node->name = (char *)NULL;
    return new_node;
}

struct AST_Node *ASTChangeTypeClassGST(struct AST_Node *root, struct TypeTable *type, struct ClassTable *class)
{
    if (root != NULL)
    {
        root->left = ASTChangeTypeClassGST(root->left, type, class);
        root->right = ASTChangeTypeClassGST(root->right, type, class);
        if (root->nodetype == VARIABLE)
        {
            GSTChangeTypeClass(root, type, class);
        }
        if (root->nodetype == ARRAY)
        {
            if (CLookup(type->name) != NULL)
            {
                printf("Error: Type mismatch. \"Array cannot be of type class\".\n");
                exit(1);
            }
            if (strcmp(type->name, "int") != 0 && strcmp(type->name, "str") != 0)
            {
                printf("Error: Type mismatch. \"Array cannot be of user defined type\".\n");
                exit(1);
            }
            GSTChangeTypeClass(root, type, class);
        }
        if (root->nodetype == FUNCTION)
        {
            if (CLookup(type->name) != NULL)
            {
                printf("Error: Type mismatch. \"Function cannot be of type class\".\n");
                exit(1);
            }
            GSTChangeTypeClass(root, type, class);
        }
        root = NULL;
        free(root);
    }
}

struct AST_Node *ASTChangeTypeLST(struct LSTable *table, struct AST_Node *root, struct TypeTable *type)
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
        if (typeCheckInt(root))
            printf("%d", root->val);
        else if (typeCheckStr(root))
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
        if (root->left != NULL)
        {
            ASTPrint(root->left);
            printf(";\n");
        }
        if (root->right != NULL)
            ASTPrint(root->right);
    }

    else if (root->nodetype == OPERATOR)
    {
        if (typeCheckBool(root))
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
        printf(";\nendwhile");
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
        printf("endif");
    }

    else if (root->nodetype == ARRAY)
    {
        printf("%s[", root->name);
        ASTPrint(root->left);
        printf("]");
    }

    else if (root->nodetype == FUNCTION)
    {
        printf("%s %s(", root->type->name, root->name);
        struct AST_Node *curr = root->right;
        while (curr)
        {
            printf("%s %s", curr->type->name, curr->s);
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
    }

    else if (root->nodetype == FUNCTIONCALL)
    {
        printf("%s(", root->name);
        struct AST_Node *curr_arg = root->arg_list;
        while (curr_arg)
        {
            ASTPrint(curr_arg);
            curr_arg = curr_arg->next_arg;
            if (curr_arg)
                printf(", ");
        }
        printf(")");
    }

    else if (root->nodetype == BREAK)
    {
        printf("break");
    }

    else if (root->nodetype == CONTINUE)
    {
        printf("continue");
    }

    else if (root->nodetype == BREAKPOINT)
    {
        printf("breakpoint");
    }

    else if (root->nodetype == RET)
    {
        printf("return ");
        ASTPrint(root->left);
        printf(";\n");
    }

    else if (root->nodetype == FIELD)
    {
        printf("%s.%s", root->left->name, root->right->name);
    }

    else if (root->nodetype == INITIALIZE)
    {
        printf("initialize()");
    }

    else if (root->nodetype == ALLOC)
    {
        printf("alloc()");
    }

    else if (root->nodetype == FREE)
    {
        printf("free(");
        ASTPrint(root->left);
        printf(")");
    }

    else if (root->nodetype == NULL__)
    {
        printf("null");
    }

    else if (root->nodetype == NEW)
    {
        printf("new(");
        ASTPrint(root->left);
        printf(")");
    }

    else if (root->nodetype == FIELD)
    {
        printf("%s.%s", root->left->name, root->right->name);
    }

    else if (root->nodetype == FIELDFUNCTIONCALL)
    {
        ASTPrint(root->left);
        printf(".%s(", root->right->name);
        struct AST_Node *curr_arg = root->right->arg_list;
        while (curr_arg)
        {
            ASTPrint(curr_arg);
            curr_arg = curr_arg->next_arg;
            if (curr_arg)
                printf(", ");
        }
        printf(")");
    }
}
//---------------------------AST-----------------------------

//---------------------------CodeGen Stack-----------------------------
void pushStack(struct AST_Node *node, struct LSTable *lst, struct ClassTable *class)
{
    struct Stack *new = (struct Stack *)malloc(sizeof(struct Stack));
    new->node = node;
    new->LST = lst;
    new->class = class;
    new->next = Shead;
    Shead = new;
}

struct Stack *popStack()
{
    if (Shead == NULL)
        return NULL;
    struct Stack *temp = Shead;
    Shead = Shead->next;
    return temp;
}

struct Stack *topStack()
{
    return Shead;
}

void printStack()
{
    struct Stack *curr = Shead;
    printf("Stack: ");
    while (curr)
    {
        printf("%s  ", curr->node->name);
        curr = curr->next;
    }
    printf("\n\n");
}
//---------------------------CodeGen Stack-----------------------------

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
        struct AST_Node *new_node = makeNode(VARIABLE, curr->type, NULL, NULL, NULL, NULL, NULL, curr->name);
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
        Phead = ParamInstall(Phead, curr->name, curr->type);
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

//---------------------------Type Checking-----------------------------
int typeCheckInt(struct AST_Node *root)
{
    if (strcmp(root->type->name, "int") == 0)
        return 1;
    return 0;
}

int typeCheckBool(struct AST_Node *root)
{
    if (strcmp(root->type->name, "boolean") == 0)
        return 1;
    return 0;
}

int typeCheckStr(struct AST_Node *root)
{
    if (strcmp(root->type->name, "str") == 0)
        return 1;
    return 0;
}

int typeCheckVoid(struct AST_Node *root)
{
    if (strcmp(root->type->name, "void") == 0)
        return 1;
    return 0;
}
//---------------------------Type Checking-----------------------------

//---------------------------Code Generation-----------------------------
int numWhile = 0;
int labelIn[100], labelOut[100];
int codeGen(struct AST_Node *t, struct LSTable *LST, struct ClassTable *class, FILE *target_file)
{
    int p, q, addrReg;

    if (t->nodetype == CONSTANT)
    {
        p = getReg();
        if (t->type == TTLookup("int"))
        {
            fprintf(target_file, "MOV R%d, %d\n", p, t->val);
        }
        else if (t->type == TTLookup("str"))
        {
            fprintf(target_file, "MOV R%d, %s\n", p, t->s);
        }
        return p;
    }

    else if (t->nodetype == VARIABLE)
    {
        p = getReg();
        addrReg = getAddr(t, LST, class, target_file);
        fprintf(target_file, "MOV R%d, [R%d]\n", p, addrReg);
        freeReg();
        return p;
    }

    else if (t->nodetype == ARRAY)
    {
        p = getReg();
        addrReg = getArrayAddr(t, LST, class, target_file);
        fprintf(target_file, "MOV R%d, [R%d]\n", p, addrReg);
        freeReg();
        return p;
    }

    else if (t->nodetype == FIELD)
    {
        p = getReg();
        addrReg = getFieldAddr(t, LST, class, target_file);
        fprintf(target_file, "MOV R%d, [R%d]\n", p, addrReg);
        freeReg();
        return p;
    }

    else if (t->nodetype == NULL__)
    {
        p = getReg();
        fprintf(target_file, "MOV R%d, -1\n", p);
        return p;
    }

    else if (t->nodetype == READ)
    {
        p = getReg();
        if (t->left->nodetype == VARIABLE)
        {
            addrReg = getAddr(t->left, LST, class, target_file);
            fprintf(target_file, "MOV R%d, R%d\n", p, addrReg);
            freeReg();
        }
        else if (t->left->nodetype == ARRAY)
        {
            addrReg = getArrayAddr(t->left, LST, class, target_file);
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
        p = codeGen(t->left, LST, class, target_file);
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
        if (t->left != NULL)
            codeGen(t->left, LST, class, target_file);
        if (t->right != NULL)
            codeGen(t->right, LST, class, target_file);
        return -1;
    }

    else if (t->nodetype == OPERATOR)
    {
        if (t->type == TTLookup("boolean"))
        {
            p = codeGen(t->left, LST, class, target_file);
            q = codeGen(t->right, LST, class, target_file);
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
            else if (strcmp(t->s, "AND") == 0)
            {
                fprintf(target_file, "MUL R%d, R%d\n", p, q);
            }
            else if (strcmp(t->s, "OR") == 0)
            {
                fprintf(target_file, "ADD R%d, R%d\n", p, q);
            }
            else if (strcmp(t->s, "NOT") == 0)
            {
                fprintf(target_file, "EQ R%d, 0\n", p);
            }
            freeReg();
            return p;
        }

        if (t->s[0] == '=')
        {
            p = codeGen(t->right, LST, class, target_file);

            if (t->left->nodetype == VARIABLE)
            {
                addrReg = getAddr(t->left, LST, class, target_file);
                fprintf(target_file, "MOV [R%d], R%d\n", addrReg, p);
                freeReg();
            }
            else if (t->left->nodetype == ARRAY)
            {
                addrReg = getArrayAddr(t->left, LST, class, target_file);
                fprintf(target_file, "MOV [R%d], R%d\n", addrReg, p);
                freeReg();
            }
            else if (t->left->nodetype == FIELD)
            {
                addrReg = getFieldAddr(t->left, LST, class, target_file);
                fprintf(target_file, "MOV [R%d], R%d\n", addrReg, p);
                freeReg();
            }
            freeReg();
            return -1;
        }

        else
        {
            p = codeGen(t->left, LST, class, target_file);
            q = codeGen(t->right, LST, class, target_file);
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
        p = codeGen(t->left, LST, class, target_file);
        fprintf(target_file, "JZ R%d, L%d\n", p, v);
        freeReg();
        p = codeGen(t->right, LST, class, target_file);
        fprintf(target_file, "JMP L%d\n", u);
        fprintf(target_file, "L%d:\n", v);
        numWhile--;
        return -1;
    }

    else if (t->nodetype == IF)
    {
        int r, s;
        s = getLabel();
        p = codeGen(t->left, LST, class, target_file);
        fprintf(target_file, "JZ R%d, L%d\n", p, s);
        freeReg();
        p = codeGen(t->mid, LST, class, target_file);
        if (t->right != NULL)
        {
            r = getLabel();
            fprintf(target_file, "JMP L%d\n", r);
            fprintf(target_file, "L%d:\n", s);
            p = codeGen(t->right, LST, class, target_file);
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

    else if (t->nodetype == BREAKPOINT)
    {
        fprintf(target_file, "BRKP\n");
        return -1;
    }

    else if (t->nodetype == INITIALIZE)
    {
        p = getReg();
        fprintf(target_file, "MOV R%d, \"Heapset\"\n", p);
        fprintf(target_file, "PUSH R%d\n", p);
        fprintf(target_file, "PUSH R%d\n", p);
        fprintf(target_file, "PUSH R%d\n", p);
        fprintf(target_file, "PUSH R%d\n", p);
        fprintf(target_file, "PUSH R%d\n", p);
        fprintf(target_file, "CALL 0\n");
        fprintf(target_file, "POP R%d\n", p);
        fprintf(target_file, "POP R%d\n", p);
        fprintf(target_file, "POP R%d\n", p);
        fprintf(target_file, "POP R%d\n", p);
        fprintf(target_file, "POP R%d\n", p);
        freeReg();
        return -1;
    }

    else if (t->nodetype == ALLOC || t->nodetype == NEW)
    {
        p = getReg();
        q = getReg();
        fprintf(target_file, "MOV R%d, \"Alloc\"\n", p);
        fprintf(target_file, "PUSH R%d\n", p);
        fprintf(target_file, "MOV R%d, 8\n", p);
        fprintf(target_file, "PUSH R%d\n", p);
        fprintf(target_file, "PUSH R%d\n", p);
        fprintf(target_file, "PUSH R%d\n", p);
        fprintf(target_file, "PUSH R%d\n", p);
        fprintf(target_file, "CALL 0\n");
        fprintf(target_file, "POP R%d\n", p);
        fprintf(target_file, "POP R%d\n", q);
        fprintf(target_file, "POP R%d\n", q);
        fprintf(target_file, "POP R%d\n", q);
        fprintf(target_file, "POP R%d\n", q);
        freeReg();
        return p;
    }

    else if (t->nodetype == FREE)
    {
        if (t->left->nodetype == VARIABLE)
            addrReg = getAddr(t->left, LST, class, target_file);
        else if (t->left->nodetype == FIELD)
            addrReg = getFieldAddr(t->left, LST, class, target_file);
        p = getReg();
        fprintf(target_file, "MOV R%d, R%d\n", p, addrReg);
        fprintf(target_file, "MOV R%d, [R%d]\n", p, p);
        q = getReg();
        fprintf(target_file, "PUSH R%d\n", p);
        fprintf(target_file, "MOV R%d, \"Free\"\n", q);
        fprintf(target_file, "PUSH R%d\n", q);
        fprintf(target_file, "PUSH R%d\n", addrReg);
        fprintf(target_file, "PUSH R%d\n", q);
        fprintf(target_file, "PUSH R%d\n", q);
        fprintf(target_file, "CALL 0\n");
        fprintf(target_file, "POP R%d\n", addrReg);
        fprintf(target_file, "POP R%d\n", q);
        fprintf(target_file, "POP R%d\n", q);
        fprintf(target_file, "POP R%d\n", q);
        fprintf(target_file, "POP R%d\n", q);
        fprintf(target_file, "POP R%d\n", p);
        freeReg();
        freeReg();
        return addrReg;
    }

    else if (t->nodetype == FUNCTION)
    {
        if (class != NULL)
        {
            struct MethodListNode *curr = CLookupMethod(class, t->name);
            fprintf(target_file, "F%d:\n", curr->mlabel);
            fprintf(target_file, "PUSH BP\n");
            fprintf(target_file, "MOV BP, SP\n");
            fprintf(target_file, "ADD SP, %d\n", LST->size - ParamGetCount(ArgToParam(t->right)) - 1);
        }
        else
        {
            struct GST_Node *curr = GSTLookup(t->name);
            fprintf(target_file, "F%d:\n", curr->flabel);
            fprintf(target_file, "PUSH BP\n");
            fprintf(target_file, "MOV BP, SP\n");
            fprintf(target_file, "ADD SP, %d\n", LST->size - ParamGetCount(ArgToParam(t->right)));
        }
        if (t->left)
            p = codeGen(t->left, LST, class, target_file);
        return -1;
    }

    else if (t->nodetype == FIELDFUNCTIONCALL)
    {
        // Push all registers in use
        int regs = free_reg - 1;
        for (int i = 0; i <= regs; i++)
        {
            fprintf(target_file, "PUSH R%d\n", i);
            freeReg();
        }
        p = codeGen(t->left, LST, class, target_file);
        fprintf(target_file, "PUSH R%d\n", p);
        freeReg();
        // Push arguments
        int numArgs = 0;
        if (t->right->arg_list)
            numArgs = pushArgs(t->right->arg_list, numArgs, LST, class, target_file);
        // Push space for return value
        p = getReg();
        fprintf(target_file, "PUSH R%d\n", p);
        freeReg();
        // Call function
        int f = CLookupMethod(t->left->class, t->right->name)->mlabel;
        fprintf(target_file, "CALL F%d\n", f);
        // Get back all registers in use
        for (int i = regs; i >= 0; i--)
            getReg();
        // Pop return value
        p = getReg();
        fprintf(target_file, "POP R%d\n", p);
        // Pop arguments
        q = getReg();
        for (int i = 0; i < numArgs + 1; i++) // +1 for self
            fprintf(target_file, "POP R%d\n", q);
        freeReg();
        // Pop registers in use
        for (int i = regs; i >= 0; i--)
            fprintf(target_file, "POP R%d\n", i);
        return p;
    }

    else if (t->nodetype == FUNCTIONCALL)
    {
        // Push all registers in use
        int regs = free_reg - 1;
        for (int i = 0; i <= regs; i++)
        {
            fprintf(target_file, "PUSH R%d\n", i);
            freeReg();
        }
        // Push arguments
        int numArgs = 0;
        if (t->arg_list)
            numArgs = pushArgs(t->arg_list, numArgs, LST, class, target_file);
        // Push space for return value
        p = getReg();
        fprintf(target_file, "PUSH R%d\n", p);
        freeReg();
        // Call function
        int f = GSTLookup(t->name)->flabel;
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
        q = codeGen(t->left, LST, class, target_file);
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
int getAddr(struct AST_Node *t, struct LSTable *LST, struct ClassTable *class, FILE *target_file)
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

int getArrayAddr(struct AST_Node *t, struct LSTable *LST, struct ClassTable *class, FILE *target_file)
{
    struct GST_Node *g = GSTLookup(t->name);
    int p = getReg();
    fprintf(target_file, "MOV R%d, %d\n", p, g->binding);
    int q = codeGen(t->left, LST, class, target_file);
    fprintf(target_file, "ADD R%d, R%d\n", p, q);
    freeReg();
    return p;
}

int getFieldAddr(struct AST_Node *t, struct LSTable *LST, struct ClassTable *class, FILE *target_file)
{
    if (t)
    {
        int a = getFieldAddr(t->left, LST, class, target_file);
        if (t->nodetype == VARIABLE)
        {
            a = getAddr(t, LST, class, target_file);
        }
        else if (t->nodetype == FIELD)
        {
            int fieldIndex;
            if (t->left->type)
                fieldIndex = TTLookupField(t->left->type, t->right->name)->fieldIndex;
            else
                fieldIndex = CLookupField(t->left->class, t->right->name)->fieldIndex;
            fprintf(target_file, "MOV R%d, [R%d]\n", a, a);
            fprintf(target_file, "ADD R%d, %d\n", a, fieldIndex);
        }
        return a;
    }
    return -1;
}

int pushArgs(struct AST_Node *root, int numArgs, struct LSTable *LST, struct ClassTable *class, FILE *target_file)
{
    if (root)
    {
        numArgs++;
        numArgs = pushArgs(root->next_arg, numArgs, LST, class, target_file);
        int p = codeGen(root, LST, class, target_file);
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
        table = LSTInstall(table, curr->name, curr->type, NULL);
        table->tail->binding = i;
        i--;
        curr = curr->next;
    }
    if (inClass)
    {
        table = LSTInstall(table, "self", NULL, Ctail);
        table->tail->binding = i;
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
