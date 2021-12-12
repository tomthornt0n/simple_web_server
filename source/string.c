
typedef struct{
    size_t len;
    char *buffer;
}String8;
#define string8_literal(S) ((String8){ sizeof(S) - 1, (S)})

static String8
cstring_as_string8(char *cstring){
    String8 result = {
        .len = strlen(cstring),
        .buffer = cstring,
    };
}

// NOTE(tbt): expands a String8 to be used with a "%.*s" format string
#define fmt_string8(S) (S).len, (S).buffer

static String8
string8_from_fmt_v(char *fmt, va_list args){
    String8 result = {0};
    va_list args_2;
    va_copy(args_2, args);
    result.len = vsnprintf(NULL, 0, fmt, args_2);
    result.buffer = calloc(result.len + 2, 1);
    vsnprintf(result.buffer, result.len + 1, fmt, args);
    return result;
}

static String8
string8_from_fmt(char *fmt, ...){
    va_list args;
    va_start(args, fmt);
    String8 result = string8_from_fmt_v(fmt, args);
    va_end(args);
    return result;
}

typedef struct String8Node String8Node;
struct String8Node{
    String8 string;
    String8Node *next;
};

typedef struct{
    String8Node *first;
    String8Node *last;
}String8List;

static void
string8_list_push(String8List *list, String8 string){
    String8Node *new_node = malloc(sizeof(*new_node));
    new_node->string = string;
    new_node->next = list->first;
    if(NULL == list->first){
        list->last = new_node;
    }
    list->first = new_node;
}

static void
string8_list_append(String8List *list, String8 string){
    String8Node *new_node = malloc(sizeof(*new_node));
    new_node->string = string;
    new_node->next = NULL;
    if(NULL == list->last){
        list->first = new_node;
    }else{
        list->last->next = new_node;
    }
    list->last = new_node;
}
