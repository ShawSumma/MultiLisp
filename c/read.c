#include "interp.h"

struct pack_ast_node;
typedef struct pack_ast_node pack_ast_node;
struct pack_ast_tagstr;
typedef struct pack_ast_tagstr pack_ast_tagstr;

struct pack_ast_node {
    enum {
        PACK_AST_NODE_NIL,
        PACK_AST_NODE_CALL,
        PACK_AST_NODE_NAME,
        PACK_AST_NODE_STRING,
        PACK_AST_NODE_NUMBER,
        PACK_AST_NODE_PROGRAM,
    } type;
    union {
        double number;
        char *string;
        char *name;
        struct {
            pack_ast_node **code;
            size_t codecount;
        } program;
        struct {
            pack_ast_node *func;
            pack_ast_node **args;
            size_t argcount;
        } call;
    } value;
};

struct pack_ast_tagstr {
    char *str;
    size_t place;
    size_t len;
};

pack_ast_tagstr *pack_ast_tagstr_new(char *str) {
    pack_ast_tagstr *ret = gc_malloc(sizeof(pack_ast_tagstr));
    ret->str = str;
    ret->place = 0;
    ret->len = strlen(str);
    return ret;
}

char pack_ast_tagstr_getchar(pack_ast_tagstr *tag) {
    if (tag->place == tag->len) {
        return EOF;
    }
    char ret = tag->str[tag->place];
    tag->place ++;
    return ret;
}

void pack_ast_dumpd(pack_ast_node *node, size_t depth) {
    for (size_t i = 0; i < depth; i++) {
        printf("  ");
    }
    switch (node->type) {
        case PACK_AST_NODE_NIL: {
            printf("(nil)\n");
            break;
        }
        case PACK_AST_NODE_CALL: {
            printf("call:\n");
            pack_ast_dumpd(node->value.call.func, depth+1);
            for (size_t i = 0; i < depth+1; i++) {
                printf("  ");
            }
            printf("args:\n");
            for (size_t i = 0; i < node->value.call.argcount; i++) {
                pack_ast_dumpd(node->value.call.args[i], depth+2);
            }
            break;
        }
        case PACK_AST_NODE_NUMBER: {
            printf("number:\n");
            for (size_t i = 0; i < depth+1; i++) {
                printf("  ");
            }
            printf("%lf\n", node->value.number);
            break;
        }
        case PACK_AST_NODE_STRING: {
            printf("string:\n");
            for (size_t i = 0; i < depth+1; i++) {
                printf("  ");
            }
            printf("\"%s\n\"", node->value.string);
            break;
        }
        case PACK_AST_NODE_NAME: {
            printf("name:\n");
            for (size_t i = 0; i < depth+1; i++) {
                printf("  ");
            }
            printf("%s\n", node->value.name);
            break;
        }
        case PACK_AST_NODE_PROGRAM: {
            printf("program:\n");
            for (size_t i = 0; i < node->value.program.codecount; i++) {
                pack_ast_dumpd(node->value.program.code[i], depth+1);
            }
            break;
        }
    }
}

void pack_ast_dump(pack_ast_node *node) {
    pack_ast_dumpd(node, 0);
}

pack_ast_node *pack_ast_read(pack_ast_tagstr *tag, char last) {
    if (last == ' ' || last == '\r' || last == '\n' || last == '\t') {
        last = pack_ast_tagstr_getchar(tag);
        return pack_ast_read(tag, last);
    }
    pack_ast_node *ret = gc_malloc(sizeof(pack_ast_node));
    if (last == '\0') {
        last = pack_ast_tagstr_getchar(tag);
        size_t alloc = 4;
        size_t count = 0;
        pack_ast_node **code = gc_malloc(sizeof(pack_value *) * alloc);
        while (last != EOF) {
            if (count + 2 > alloc) {
                alloc *= 2;
                code = gc_realloc(code, sizeof(pack_value *) * alloc);
            }
            code[count] = pack_ast_read(tag, last);
            count ++;
            last = pack_ast_tagstr_getchar(tag);
        }
        ret->type = PACK_AST_NODE_PROGRAM;
        ret->value.program.code = code;
        ret->value.program.codecount = count;
    }
    else if (last == '(' || last == '[') {
        do {
            last = pack_ast_tagstr_getchar(tag);
        } while (last == ' ' || last == '\r' || last == '\n' || last == '\t');
        if (last == ')') {
            ret->type = PACK_AST_NODE_NIL;
            goto ret;
        }
        size_t alloc = 4;
        size_t count = 0;
        last = pack_ast_tagstr_getchar(tag);
        pack_ast_node **code = gc_malloc(sizeof(pack_value *) * alloc);
        while (last != ')' || last == ']') {
            if (count + 2 > alloc) {
                alloc *= 2;
                code = gc_realloc(code, sizeof(pack_value *) * alloc);                
            }
            code[count] = pack_ast_read(tag, last);
            count ++;
            do {
                last = pack_ast_tagstr_getchar(tag);
            } while (last == ' ' || last == '\r' || last == '\n' || last == '\t');
        }
        ret->type = PACK_AST_NODE_CALL;
        ret->value.call.func = code[0];
        ret->value.call.args = code+1;
        ret->value.call.argcount = count-1;
    }
    else if (last >= '0' && last <= '9') {
        size_t alloc = 8;
        size_t place = 0;
        char *value = gc_malloc(sizeof(char) * alloc);
        while ((last >= '0' && last <= '9') || last == '.') {
            if (place + 2 > alloc) {
                alloc *= 2;
                value = gc_realloc(value, sizeof(char) * alloc);
            }
            value[place] = last;
            place ++;
            last = pack_ast_tagstr_getchar(tag);
        }
        tag->place --;
        value[place] = '\0';
        ret->type = PACK_AST_NODE_NUMBER;
        ret->value.number = atof(value);
    }
    else if (last == '"') {
        last = pack_ast_tagstr_getchar(tag);
        size_t alloc = 16;
        size_t place = 0;
        char *str = gc_malloc(sizeof(char) * alloc);
        while (last != '"') {
            if (place + 2 > alloc) {
                alloc *= 2;
                str = gc_realloc(str, sizeof(char) * alloc);
            }
            str[place] = last;
            place ++; 
            last = pack_ast_tagstr_getchar(tag);
        }
        str[place] = '\0';
        ret->type = PACK_AST_NODE_STRING;
        ret->value.string = str;
    }
    else {
        size_t alloc = 8;
        size_t place = 0;
        char *name = malloc(sizeof(char) * alloc);
        while (last != '"' && last != '(' && last != ')' && last != ']' && last != '[' &&
            last != ' ' && last != '\t' && last != '\n'  && last == '\r'
            && last != EOF) {
            if (place + 2 > alloc) {
                alloc *= 2;
                name = gc_realloc(name, sizeof(char) * alloc);
            }
            name[place] = last;
            place ++;
            last = pack_ast_tagstr_getchar(tag);
        }
        name[place] = '\0';
        tag->place --;
        ret->type = PACK_AST_NODE_NAME;
        ret->value.name = name;
    }
    ret:
    return ret;
}

int main() {
    pack_ast_dump(pack_ast_read(pack_ast_tagstr_new("(cat + *)"), '\0'));
}