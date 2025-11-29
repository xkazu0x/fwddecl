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
parse_struct(Tokenizer *tokenizer) {
  Token token = get_token(tokenizer);
  if (token.type == TOKEN_IDENTIFIER) {
    Temp scratch = scratch_begin(0, 0);
    String8 result = str8_fmt(scratch.arena,
                              "typedef struct %.*s %.*s;",
                              token.text.len, token.text.str,
                              token.text.len, token.text.str);
    log_info("%s", result.str);
    scratch_end(scratch);
  }
}

internal void
parse_internal(Tokenizer *tokenizer) {
  Temp scratch = scratch_begin(0, 0);

  Token_List token_list = {0};
  for (;;) {
    Token token = get_token(tokenizer);
    if (token.type != TOKEN_OPEN_BRACE) {
      Token_Node *token_node = push_array(scratch.arena, Token_Node, 1);
      token_node->token = token;
      dll_push_back(token_list.first, token_list.last, token_node);
    } else {
      break;
    }
  }

  String8_List string_list = {0};
  str8_list_push(scratch.arena, &string_list, str8_lit("internal "));

  for (Token_Node *node = token_list.first; node != 0; node = node->next) {
    Token token = node->token;
    switch (token.type) {
      case TOKEN_IDENTIFIER: {
        Token next_token = node->next->token;
        if (next_token.type == TOKEN_IDENTIFIER ||
            next_token.type == TOKEN_ASTERISK) {
          str8_list_push_fmt(scratch.arena, &string_list, "%.*s ",
                             token.text.len, token.text.str);
        } else {
          str8_list_push(scratch.arena, &string_list, token.text);
        }
      } break;

      case TOKEN_COMMA: {
        str8_list_push_fmt(scratch.arena, &string_list, "%.*s ",
                           token.text.len, token.text.str);
      } break;

      default: {
        str8_list_push(scratch.arena, &string_list, token.text);
      } break;
    }
  }

  str8_list_push(scratch.arena, &string_list, str8_lit(";"));

  uxx total_len = 0;
  for (String8_Node *node = string_list.first; node != 0; node = node->next) {
    String8 string = node->str;
    total_len += string.len;
  }

  u8 *str = arena_push(scratch.arena, total_len + 1);
  u8 *ptr = str;
  for (String8_Node *node = string_list.first; node != 0; node = node->next) {
    String8 string = node->str;
    mem_copy(ptr, string.str, string.len);
    ptr += string.len;
  }
  str[total_len] = 0;

  String8 result = {
    .len = total_len,
    .str = str,
  };

  log_info("%s", result.str);

  scratch_end(scratch);
}

int
main(void) {
  Thread_Context *thread_context = thread_context_alloc();
  thread_context_select(thread_context);

  Arena *arena = arena_alloc();
  String8 src_file_path = str8_lit("..\\src\\example.c");
  log_info("input: %.*s", src_file_path.len, src_file_path.str);

  uxx last_dot = str8_find_last(src_file_path, '.');
  String8 dst_file_name = str8_substr(src_file_path, 0, last_dot);
  dst_file_name = str8_cat(arena, dst_file_name, str8_lit(".meta.h"));
  log_info("output: %.*s", dst_file_name.len, dst_file_name.str);

  String8 file_data = string_from_file_path(arena, src_file_path);
  Tokenizer tokenizer = {.at = file_data.str};

  for (b32 stop = false; !stop;) {
    Token token = get_token(&tokenizer);
    switch (token.type) {
      case TOKEN_END_OF_FILE: {
        stop = true;
      } break;

      case TOKEN_UNKNOWN: {
      } break;

      case TOKEN_IDENTIFIER: {
        if (token_match_text(token, "struct")) {
          parse_struct(&tokenizer);
        } else if (token_match_text(token, "internal")) {
          parse_internal(&tokenizer);
        }
      } break;

      default: {
        // log_info("%d: %.*s", token.type, token.text.len, token.text.str);
      }
    }
  }

  return(0);
}
