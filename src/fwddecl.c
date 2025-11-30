#include "base/krueger_base.h"
#include "platform/krueger_platform.h"

#include "base/krueger_base.c"
#include "platform/krueger_platform.c"

internal String8
string_from_file_path(Arena *arena, String8 file_path) {
  String8 result = {0};
  Platform_File_Flags flags = PLATFORM_FILE_READ|PLATFORM_FILE_SHARE_READ;
  Platform_Handle file = platform_file_open(file_path, flags);
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

internal void
write_string(Platform_Handle file, String8 string) {
  platform_file_write(file, string.str, string.len);
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

internal void
parse_struct(Tokenizer *tokenizer, Arena *arena, String8_List *list) {
  Token token = get_token(tokenizer);
  if (token.type == TOKEN_IDENTIFIER) {
    str8_list_push_fmt(arena, list, "typedef struct %.*s %.*s;",
                       token.text.len, token.text.str,
                       token.text.len, token.text.str);
    str8_list_push_copy(arena, list, str8_lit("\n"));
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
  str8_list_push_copy(scratch.arena, &string_list, str8_lit("internal "));
  for (each_node(Token_Node, node, token_list.first)) {
    Token token = node->token;
    String8 text = token.text;

    switch (token.type) {
      case TOKEN_IDENTIFIER: {
        Token next_token = node->next->token;
        if (next_token.type == TOKEN_IDENTIFIER ||
            next_token.type == TOKEN_ASTERISK) {
          str8_list_push_fmt(scratch.arena, &string_list, "%.*s ",
                             (int)text.len, (char *)text.str);
        } else {
          str8_list_push(scratch.arena, &string_list, text);
        }
      } break;

      case TOKEN_COMMA: {
        str8_list_push_fmt(scratch.arena, &string_list, "%.*s ",
                           (int)text.len, (char *)text.str);
      } break;

      default: {
        str8_list_push(scratch.arena, &string_list, text);
      } break;
    }
  }
  str8_list_push_copy(scratch.arena, &string_list, str8_lit(";"));

  String8 string = str8_list_join(arena, &string_list);
  str8_list_push(arena, list, string);
  str8_list_push_copy(arena, list, str8_lit("\n"));

  scratch_end(scratch);
}

internal void
entry_point(int argc, char **argv) {
  if (argc != 2) {
    log_error("usage > fwddecl <file_path>");
    platform_abort(1);
  }

  Arena *arena = arena_alloc();
  String8 src_file_path = str8_cstr(argv[1]);

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
        if (str8_match_lit(token.text, "struct")) {
          parse_struct(&tokenizer, struct_arena, &struct_list);
        } else if (str8_match_lit(token.text, "internal")) {
          parse_internal(&tokenizer, proc_arena, &proc_list);
        }
      } break;
    }
  }

  String8 post_fix = str8_lit(".meta.h");
  uxx last_slash = str8_find_last(src_file_path, '\\');
  uxx last_dot = str8_find_last(src_file_path, '.');
  
  String8 dst_file_path = str8_substr(src_file_path, 0, last_dot);
  dst_file_path = str8_cat(arena, dst_file_path, post_fix);

  String8 file_name = str8_substr(src_file_path, last_slash + 1, last_dot);
  file_name = str8_cat(arena, file_name, post_fix);

  Platform_Handle file = platform_file_open(dst_file_path, PLATFORM_FILE_WRITE); {
    String8 guard = {0};
    guard.len = file_name.len;
    guard.str = push_array(arena, u8, guard.len + 1);
    guard.str[guard.len] = 0;

    for (uxx i = 0; i < file_name.len; ++i) {
      u8 c = file_name.str[i];
      if (char_is_lower(c)) {
        c -= 'a' - 'A';
      } else if (c == '.') {
        c = '_';
      }
      guard.str[i] = c;
    }

    String8 string = str8_cat(arena, str8_lit("#ifndef "), guard);
    write_string(file, string);
    write_string(file, str8_lit("\n"));

    string = str8_cat(arena, str8_lit("#define "), guard);
    write_string(file, string);
    write_string(file, str8_lit("\n\n"));

    for (each_node(String8_Node, node, struct_list.first)) {
      write_string(file, node->string);
    }
    write_string(file, str8_lit("\n"));

    for (each_node(String8_Node, node, proc_list.first)) {
      write_string(file, node->string);
    }
    write_string(file, str8_lit("\n"));

    string = str8_cat(arena, str8_lit("#endif // "), guard);
    platform_file_write(file, string.str, string.len);
  } platform_file_close(file);
}
