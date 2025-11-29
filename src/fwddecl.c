#include "base/krueger_base.h"
#include "platform/krueger_platform.h"

#include "base/krueger_base.c"
#include "platform/krueger_platform.c"

internal String8
string_from_file_path(Arena *arena, String8 file_path) {
  String8 result = {0};
  Platform_Handle file = platform_file_open(file_path, PLATFORM_FILE_READ | PLATFORM_FILE_SHARE_READ);
  if (!platform_handle_is_null(file)) {
    u64 file_size = platform_get_file_size(file);
    result.len = file_size;
    result.str = arena_push(arena, file_size + 1);
    platform_file_read(file, result.str, file_size);
    result.str[result.len] = 0;
    platform_file_close(file);
  }
  return(result);
}

typedef enum {
  TOKEN_UNKNOWN,

  TOKEN_OPEN_PAREN,
  TOKEN_CLOSE_PAREN,
  TOKEN_OPEN_BRACKET,
  TOKEN_CLOSE_BRACKET,
  TOKEN_OPEN_BRACE,
  TOKEN_CLOSE_BRACE,
  TOKEN_DOT,
  TOKEN_COMMA,
  TOKEN_COLON,
  TOKEN_SEMICOLON,
  TOKEN_ASTERISK,

  TOKEN_IDENTIFIER,

  TOKEN_END_OF_FILE,
} Token_Type;

typedef struct {
  Token_Type type;
  String8 text;
} Token;

typedef struct Token_Node Token_Node;
struct Token_Node {
  Token_Node *prev;
  Token_Node *next;
  Token token;
};

typedef struct {
  Token_Node *first;
  Token_Node *last;
} Token_List;

typedef struct {
  u8 *at;
} Tokenizer;

internal b32
char_is_end_of_line(u8 c) {
  b32 result = ((c == '\r') || (c == '\n'));
  return(result);
}

internal Token
get_token(Tokenizer *tokenizer) {
  for (;;) {
    if (char_is_space(tokenizer->at[0])) {
      ++tokenizer->at;
    } else if (tokenizer->at[0] == '/' &&
               tokenizer->at[1] == '/') {
      tokenizer->at += 2;
      while (tokenizer->at[0] && !char_is_end_of_line(tokenizer->at[0])) {
        ++tokenizer->at;
      }
    } else if (tokenizer->at[0] == '/' &&
               tokenizer->at[1] == '*') {
      tokenizer->at += 2;
      while (tokenizer->at[0] &&
             !(tokenizer->at[0] == '*' &&
               tokenizer->at[1] == '/')) {
        ++tokenizer->at;
      }
      if (tokenizer->at[0] == '*') {
        tokenizer->at += 2;
      }
    } else {
      break;
    }
  }

  Token_Type type = TOKEN_UNKNOWN;
  u8 *start = tokenizer->at;

  u8 c = *tokenizer->at++;
  switch (c) {
    case '\0': {type = TOKEN_END_OF_FILE;} break;

    case '(': {type = TOKEN_OPEN_PAREN;} break;
    case ')': {type = TOKEN_CLOSE_PAREN;} break;
    case '[': {type = TOKEN_OPEN_BRACKET;} break;
    case ']': {type = TOKEN_CLOSE_BRACKET;} break;
    case '{': {type = TOKEN_OPEN_BRACE;} break;
    case '}': {type = TOKEN_CLOSE_BRACE;} break;
    case '.': {type = TOKEN_DOT;} break; 
    case ',': {type = TOKEN_COMMA;} break;
    case ':': {type = TOKEN_COLON;} break; 
    case ';': {type = TOKEN_SEMICOLON;} break;
    case '*': {type = TOKEN_ASTERISK;} break;

    default: {
      if (char_is_alpha(c)) {
        type = TOKEN_IDENTIFIER;
        while (char_is_alpha(tokenizer->at[0]) ||
               char_is_digit(tokenizer->at[0]) ||
               tokenizer->at[0] == '_') {
          ++tokenizer->at;
        }
      }
    } break;
  }

  u8 *end = tokenizer->at;

  Token token = {
    token.type = type,
    token.text = str8_range(start, end),
  };

  return(token);
}

internal b32
token_match_text(Token token, char *cstr) {
  b32 result = str8_match_cstr(token.text, cstr);
  return(result);
}

internal void
parse_struct(Tokenizer *tokenizer, Arena *arena, String8_List *list) {
  Token token = get_token(tokenizer);
  if (token.type == TOKEN_IDENTIFIER) {
    String8 copy = str8_fmt(arena, "typedef struct %.*s %.*s;",
                            token.text.len, token.text.str,
                            token.text.len, token.text.str);
    str8_list_push(arena, list, copy);

    copy = str8_copy(arena, str8_lit("\n"));
    str8_list_push(arena, list, copy);
  }
}

internal void
parse_internal(Tokenizer *tokenizer, Arena *arena, String8_List *list) {
  Temp scratch = scratch_begin(&arena, 1);
  Token_List token_list = {0};

  for (;;) {
    Token token = get_token(tokenizer);
    if (token.type == TOKEN_OPEN_BRACE) break;

    Token_Node *token_node = push_array(scratch.arena, Token_Node, 1);
    token_node->token = token;
    dll_push_back(token_list.first, token_list.last, token_node);
  }

  String8_List string_list = {0};
  String8 copy = str8_copy(scratch.arena, str8_lit("internal "));
  str8_list_push(scratch.arena, &string_list, copy);

  for (Token_Node *node = token_list.first; node != 0; node = node->next) {
    Token token = node->token;
    String8 text = token.text;

    switch (token.type) {
      case TOKEN_IDENTIFIER: {
        Token next_token = node->next->token;
        if (next_token.type == TOKEN_IDENTIFIER ||
            next_token.type == TOKEN_ASTERISK) {
          copy = str8_fmt(scratch.arena, "%.*s ", (int)text.len, (char *)text.str);
          str8_list_push(scratch.arena, &string_list, copy);
        } else {
          copy = str8_fmt(scratch.arena, "%.*s", (int)text.len, (char *)text.str);
          str8_list_push(scratch.arena, &string_list, copy);
        }
      } break;

      case TOKEN_COMMA: {
        copy = str8_fmt(scratch.arena, "%.*s ", (int)text.len, (char *)text.str);
        str8_list_push(scratch.arena, &string_list, copy);
      } break;

      default: {
        copy = str8_fmt(scratch.arena, "%.*s", (int)text.len, (char *)text.str);
        str8_list_push(scratch.arena, &string_list, copy);
      } break;
    }
  }

  copy = str8_copy(scratch.arena, str8_lit(";"));
  str8_list_push(scratch.arena, &string_list, copy);

  String8 result = {0};
  result.len = string_list.total_size;
  result.str = push_array(arena, u8, result.len + 1);
  result.str[result.len] = 0;

  u8 *ptr = result.str;
  for (String8_Node *node = string_list.first; node != 0; node = node->next) {
    String8 string = node->str;
    mem_copy(ptr, string.str, string.len);
    ptr += string.len;
  }

  str8_list_push(arena, list, result);

  copy = str8_copy(arena, str8_lit("\n"));
  str8_list_push(arena, list, copy);

  scratch_end(scratch);
}

int
main(void) {
  Thread_Context *thread_context = thread_context_alloc();
  thread_context_select(thread_context);

  Arena *arena = arena_alloc();
  String8 src_file_path = str8_lit("..\\src\\example.c");

  String8 file_data = string_from_file_path(arena, src_file_path);
  Tokenizer tokenizer = {.at = file_data.str};

  Arena *struct_arena = arena_alloc();
  String8_List struct_list = {0};

  Arena *proc_arena = arena_alloc();
  String8_List proc_list = {0};

  for (b32 stop = false; !stop;) {
    Token token = get_token(&tokenizer);
    switch (token.type) {
      case TOKEN_END_OF_FILE: {
        stop = true;
      } break;

      case TOKEN_IDENTIFIER: {
        if (token_match_text(token, "struct")) {
          parse_struct(&tokenizer, struct_arena, &struct_list);
        } else if (token_match_text(token, "internal")) {
          parse_internal(&tokenizer, proc_arena, &proc_list);
        }
      } break;
    }
  }

  uxx last_slash = str8_find_last(src_file_path, '\\');
  uxx last_dot = str8_find_last(src_file_path, '.');
  
  String8 dst_file_path = str8_substr(src_file_path, 0, last_dot);
  String8 file_name = str8_substr(src_file_path, last_slash + 1, last_dot);

  String8 post_fix = str8_lit(".meta.h");
  dst_file_path = str8_cat(arena, dst_file_path, post_fix);
  String8 dst_file_name = str8_cat(arena, file_name, post_fix);

  String8 guard = {0};
  guard.len = dst_file_name.len;
  guard.str = push_array(arena, u8, guard.len + 1);
  guard.str[guard.len] = 0;

  for (uxx i = 0; i < dst_file_name.len; ++i) {
    u8 c = dst_file_name.str[i];
    if (char_is_lower(c)) {
      c -= 'a' - 'A';
    } else if (c == '.') {
      c = '_';
    }
    guard.str[i] = c;
  }

  Platform_Handle file = platform_file_open(dst_file_path, PLATFORM_FILE_WRITE);

  String8 string = str8_cat(arena, str8_lit("#ifndef "), guard);
  platform_file_write(file, string.str, string.len);

  string = str8_lit("\n");
  platform_file_write(file, string.str, string.len);

  string = str8_cat(arena, str8_lit("#define "), guard);
  platform_file_write(file, string.str, string.len);

  string = str8_lit("\n\n");
  platform_file_write(file, string.str, string.len);

  for (String8_Node *node = struct_list.first; node != 0; node = node->next) {
    string = node->str;
    platform_file_write(file, string.str, string.len);
  }

  string = str8_lit("\n");
  platform_file_write(file, string.str, string.len);

  for (String8_Node *node = proc_list.first; node != 0; node = node->next) {
    string = node->str;
    platform_file_write(file, string.str, string.len);
  }

  string = str8_lit("\n");
  platform_file_write(file, string.str, string.len);

  string = str8_cat(arena, str8_lit("#endif // "), guard);
  platform_file_write(file, string.str, string.len);

  platform_file_close(file);

  return(0);
}
