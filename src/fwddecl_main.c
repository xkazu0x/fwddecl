#define BUILD_CONSOLE_INTERFACE 1

#include "base/krueger_base.h"
#include "os/krueger_os.h"

#include "base/krueger_base.c"
#include "os/krueger_os.c"

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
token_next(Tokenizer *tokenizer) {
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
parse_enum(Tokenizer *tokenizer, Arena *arena, String8_List *list) {
  Token type = token_next(tokenizer);
  if (type.type == TOKEN_IDENTIFIER) {
    str8_list_push_fmt(arena, list,
                       "typedef enum %.*s %.*s;\n",
                       type.text.len, type.text.str,
                       type.text.len, type.text.str);
  }
}

internal void
parse_struct(Tokenizer *tokenizer, Arena *arena, String8_List *list) {
  Token type = token_next(tokenizer);
  if (type.type == TOKEN_IDENTIFIER) {
    str8_list_push_fmt(arena, list,
                       "typedef struct %.*s %.*s;\n",
                       type.text.len, type.text.str,
                       type.text.len, type.text.str);
  }
}

internal void
parse_internal(Tokenizer *tokenizer, Arena *arena, String8_List *list) {
  Temp scratch = scratch_begin(&arena, 1);

#if 0
  String8_List strings = {0};
  str8_list_push(scratch.arena, &strings, str8_lit("internal"));
  for (;;) {
    Token token = token_next(tokenizer);
    if (token.type == TOKEN_OPEN_BRACE) break;
    str8_list_push(scratch.arena, &strings, token.text);
  }
  str8_list_push(scratch.arena, &strings, str8_lit(";\n"));
  String8 string = str8_list_join(arena, &strings, &(String_Join){.sep = str8_lit(" ")});
  str8_list_push(arena, list, string);
#else
  Token_List tokens = {0};
  for (;;) {
    Token token = token_next(tokenizer);
    if (token.type == TOKEN_OPEN_BRACE) break;
    Token_Node *node = push_array(scratch.arena, Token_Node, 1);
    node->token = token;
    dll_push_back(tokens.first, tokens.last, node);
  }

  String8_List strings = {0};
  str8_list_push(scratch.arena, &strings, str8_lit("internal "));

  for (Token_Node *node = tokens.first; node != 0; node = node->next) {
    Token token = node->token;
    String8 text = token.text;
    str8_list_push(scratch.arena, &strings, text);
    b32 put_space = false;
    if (token.type == TOKEN_IDENTIFIER) {
      Token next = node->next->token;
      if (next.type == TOKEN_IDENTIFIER ||
          next.type == TOKEN_ASTERISK) {
        put_space = true;
      }
    } else if (token.type == TOKEN_COMMA) {
      put_space = true;
    }
    if (put_space) {
      str8_list_push(scratch.arena, &strings, str8_lit(" "));
    }
  }

  str8_list_push(scratch.arena, &strings, str8_lit(";\n"));
  String8 string = str8_list_join(arena, &strings, 0);
  str8_list_push(arena, list, string);
#endif

  scratch_end(scratch);
}

internal String8
path_replace_file_extension(Arena *arena, String8 file_name, String8 ext) {
  String8 file_name_no_ext = str8_chop_last_dot(file_name);
  String8 result = str8_cat(arena, file_name_no_ext, ext);
  return(result);
}

internal String8
build_file_guard(Arena *arena, String8 file_name) {
  String8 result = {0};
  result.len = file_name.len;
  result.str = push_array(arena, u8, result.len + 1);
  result.str[result.len] = 0;
  for (uxx idx = 0; idx < file_name.len; idx += 1) {
    u8 c = file_name.str[idx];
    if (char_is_lower(c)) {
      c -= 'a' - 'A';
    } else if (c == '.') {
      c = '_';
    }
    result.str[idx] = c;
  }
  return(result);
}

internal void
str8_list_cat(String8_List *dst, String8_List *src) {
  if (src->count != 0) {
    if (dst->last) {
      dst->count += src->count;
      dst->size += src->size;
      dst->last->next = src->first;
      dst->last = src->last;
    } else {
      *dst = *src;
    }
    mem_zero_struct(src);
  }
}

internal void
entry_point(int argc, char **argv) {
  if (argc < 2) {
    log_info("usage: fwddecl <files>");
    os_abort(1);
  }

  Arena *src_arena = arena_alloc(.res_size = MB(256));
  Arena *dst_arena = arena_alloc(.res_size = MB(256));

  for (int i = 1; i < argc; i += 1) {
    Temp src_tmp = temp_begin(src_arena);
    Temp dst_tmp = temp_begin(dst_arena);

    String8 src_file_path = str8_cstr(argv[i]);
    String8 file_data = os_data_from_file_path(src_tmp.arena, src_file_path);

    if (file_data.str) {
      String8_List enums = {0};
      String8_List structs = {0};
      String8_List functions = {0};
      b32 skip_next = false;

      Tokenizer tokenizer = {.at = file_data.str};
      for (b32 stop = false; !stop;) {
        Token token = token_next(&tokenizer);
        switch (token.type) {
          case TOKEN_END_OF_FILE: {
            stop = true;
          } break;
          case TOKEN_IDENTIFIER: {
            if (skip_next) {
              skip_next = false;
              continue;
            }

            if (str8_match(token.text, str8_lit("fwddecl_ignore"))) {
              skip_next = true;
              continue;
            }

            if (str8_match(token.text, str8_lit("enum"))) {
              parse_enum(&tokenizer, dst_tmp.arena, &enums);
            } else if (str8_match(token.text, str8_lit("struct"))) {
              parse_struct(&tokenizer, dst_tmp.arena, &structs);
            } else if (str8_match(token.text, str8_lit("internal"))) {
              parse_internal(&tokenizer, dst_tmp.arena, &functions);
            }
          } break;
        }
      }

      String8 dst_file_path = path_replace_file_extension(dst_tmp.arena, src_file_path, str8_lit(".meta.h"));
      String8 file_name = str8_skip_last_slash(dst_file_path);
      String8 file_guard = build_file_guard(dst_tmp.arena, file_name);

      String8_List write_list = {0};
      str8_list_push(dst_tmp.arena, &write_list, str8_cat(dst_tmp.arena, str8_lit("#ifndef "), file_guard));
      str8_list_push(dst_tmp.arena, &write_list, str8_lit("\n"));
      str8_list_push(dst_tmp.arena, &write_list, str8_cat(dst_tmp.arena, str8_lit("#define "), file_guard));
      str8_list_push(dst_tmp.arena, &write_list, str8_lit("\n\n"));
      str8_list_push(dst_tmp.arena, &write_list, str8_lit("#ifndef fwddecl_ignore\n"));
      str8_list_push(dst_tmp.arena, &write_list, str8_lit("#define fwddecl_ignore\n"));
      str8_list_push(dst_tmp.arena, &write_list, str8_lit("#endif\n"));
      str8_list_push(dst_tmp.arena, &write_list, str8_lit("\n"));
      if (enums.count) {
        str8_list_cat(&write_list, &enums);
        str8_list_push(dst_tmp.arena, &write_list, str8_lit("\n"));
      }
      if (structs.count) {
        str8_list_cat(&write_list, &structs);
        str8_list_push(dst_tmp.arena, &write_list, str8_lit("\n"));
      }
      if (functions.count) {
        str8_list_cat(&write_list, &functions);
        str8_list_push(dst_tmp.arena, &write_list, str8_lit("\n"));
      }
      str8_list_push(dst_tmp.arena, &write_list, str8_cat(dst_tmp.arena, str8_lit("#endif // "), file_guard));

      String8 write_data = str8_list_join(dst_tmp.arena, &write_list, 0);
      os_write_data_to_file_path(dst_file_path, write_data);
    } else {
      log_error("fn %s: failed to read file: %s", __func__, src_file_path.str);
    }

    temp_end(dst_tmp);
    temp_end(src_tmp);
  }
}
