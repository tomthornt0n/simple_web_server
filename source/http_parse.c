
typedef enum{
    HTTP_Method_Options,
    HTTP_Method_Get,
    HTTP_Method_Head,
    HTTP_Method_Post,
    HTTP_Method_Put,
    HTTP_Method_Delete,
    HTTP_Method_Trace,
    HTTP_Method_Connect,
    HTTP_Method_Extension,
    HTTP_Method_MAX,
}HTTP_Method;

typedef struct HTTP_HeaderField HTTP_HeaderField;
struct HTTP_HeaderField{
    HTTP_HeaderField *next;
    String8 field;
    String8 value;
};

typedef struct{
    bool was_parse_error;
    String8 method;
    String8 uri;
    String8 version;
    HTTP_HeaderField *header_fields;
} HTTP_Request;

static HTTP_Method
http_method_from_string8(String8 string){
    HTTP_Method result;
    if(strncmp(string.buffer, "OPTIONS", string.len)){
        result = HTTP_Method_Options;
    }else if(strncmp(string.buffer, "GET", string.len)){
        result = HTTP_Method_Get;
    }else if(strncmp(string.buffer, "HEAD", string.len)){
        result = HTTP_Method_Head;
    }else if(strncmp(string.buffer, "POST", string.len)){
        result = HTTP_Method_Post;
    }else if(strncmp(string.buffer, "PUT", string.len)){
        result = HTTP_Method_Put;
    }else if(strncmp(string.buffer, "DELETE", string.len)){
        result = HTTP_Method_Delete;
    }else if(strncmp(string.buffer, "TRACE", string.len)){
        result = HTTP_Method_Trace;
    }else if(strncmp(string.buffer, "CONNECT", string.len)){
        result = HTTP_Method_Connect;
    }else{
        result = HTTP_Method_Extension;
    }
    return result;
}

static bool
http_consume_crlf_(char **cursor){
    const char *crlf = "\r\n";
    char *buffer = *cursor;
    for(int i = 0; i < 2; i += 1){
        if(buffer[i] != crlf[i]){
            return false;
        }
        *cursor += 1;
    }
    return true;
}

static HTTP_Request
http_parse_request(String8 request){
    HTTP_Request result = {0};
    
    char *cursor = request.buffer;
    
    //-NOTE(tbt): parse method token
    result.method.buffer = cursor;
    for(;' ' != *cursor;){
        result.method.len += 1;
        cursor += 1;
    }
    cursor += 1;
    
    //-NOTE(tbt): parse uri
    result.uri.buffer = cursor;
    for(;' ' != *cursor;){
        result.uri.len += 1;
        cursor += 1;
    }
    cursor += 1;
    
    // NOTE(tbt): parse protocol version
    result.version.buffer = cursor;
    for(;'\r' != *cursor;){
        result.version.len += 1;
        cursor += 1;
    }
    
    if(!http_consume_crlf_(&cursor)){
        result.was_parse_error = true;
    }
    
    //-NOTE(tbt): parse header fields
    
    
    
    return result;
}

static void
http_free_request(HTTP_Request *request){
    // TODO(tbt): implement me!
}
