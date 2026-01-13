#ifndef KRUEGER_BASE_STRING_H
#define KRUEGER_BASE_STRING_H

/////////////////////
// NOTE: String Types

typedef struct String8 String8;
struct String8 {
  uxx len;
  u8 *str;
};
typedef struct String16 String16;
struct String16 {
  uxx len;
  u16 *str;
};

typedef struct String32 String32;
struct String32 {
  uxx len;
  u32 *str;
};

////////////////////
// NOTE: String List

typedef struct String8_Node String8_Node;
struct String8_Node {
  String8_Node *next;
  String8 string;
};

typedef struct String8_List String8_List;
struct String8_List {
  String8_Node *first;
  String8_Node *last;
  u32 count;
  uxx size;
};

//////////////////////////
// NOTE: String Join Types

typedef enum Path_Style {
  PATH_STYLE_NULL,
  PATH_STYLE_WINDOWS,
  PATH_STYLE_UNIX,
#if PLATFORM_WINDOWS
  PATH_STYLE_SYSTEM = PATH_STYLE_WINDOWS,
#elif PLATFORM_LINUX || PLATFORM_MAC
  PATH_STYLE_SYSTEM = PATH_STYLE_UNIX,
#endif
} Path_Style;

typedef struct String_Join String_Join;
struct String_Join {
  String8 pre;
  String8 sep;
  String8 post;
};

///////////////////////////
// NOTE: UTF Decoding Types

typedef struct Unicode_Decode Unicode_Decode;
struct Unicode_Decode {
  u32 inc;
  u32 codepoint;
};

//////////////////////////
// NOTE: Character Helpers

internal b32 char_is_space(u8 c);
internal b32 char_is_upper(u8 c);
internal b32 char_is_lower(u8 c);
internal b32 char_is_alpha(u8 c);
internal b32 char_is_digit(u8 c);
internal b32 char_is_slash(u8 c);

/////////////////////////
// NOTE: C-String Helpers

internal uxx cstr8_len(u8 *cstr);
internal uxx cstr16_len(u16 *cstr);
internal uxx cstr32_len(u32 *cstr);

////////////////////////////
// NOTE: String Constructors

#define str8_lit(str) str8((u8 *)(str), sizeof(str) - 1)

internal String8 str8(u8 *str, uxx len);
internal String16 str16(u16 *str, uxx len);
internal String32 str32(u32 *str, uxx len);

internal String8 str8_cstr(char *cstr);
internal String16 str16_cstr(u16 *cstr);
internal String32 str32_cstr(u32 *cstr);

internal String8 str8_range(u8 *first, u8 *opl);
internal String16 str16_range(u16 *first, u16 *opl);
internal String32 str32_range(u32 *first, u32 *opl);

////////////////////////
// NOTE: String Matching

internal b32 str8_match(String8 a, String8 b);
internal uxx str8_find_first(String8 string, u8 c);
internal uxx str8_find_last(String8 string, u8 c);

///////////////////////
// NOTE: String Slicing

internal String8 str8_substr(String8 string, uxx min, uxx max);
internal String8 str8_skip(String8 string, uxx amt);
internal String8 str8_chop(String8 string, uxx amt);
internal String8 str8_prefix(String8 string, uxx size);
internal String8 str8_postfix(String8 string, uxx size);

////////////////////////////////////
// NOTE: String Formatting & Copying

internal String8 str8_cat(Arena *arena, String8 a, String8 b);
internal String8 str8_copy(Arena *arena, String8 string);
internal String8 str8_fmt_args(Arena *arena, char *fmt, va_list args);
internal String8 str8_fmt(Arena *arena, char *fmt, ...);

#define push_str8_cat(arena, a, b)    str8_cat((arena), (a), (b))
#define push_str8_copy(arena, string) str8_copy((arena), (string))
#define push_str8_fmt(arena, ...)     str8_fmt((arena), __VA_ARGS__)

////////////////////
// NOTE: String List

internal String8_Node *str8_list_push_node(String8_List *list, String8_Node *node);
internal String8_Node *str8_list_push_node_set_string(String8_List *list, String8_Node *node, String8 string);
internal String8_Node *str8_list_push(Arena *arena, String8_List *list, String8 string);
internal String8_Node *str8_list_push_copy(Arena *arena, String8_List *list, String8 string);
internal String8_Node *str8_list_push_fmt(Arena *arena, String8_List *list, char *fmt, ...);
internal String8_List str8_list_copy(Arena *arena, String8_List *list);

///////////////////////////////////
// NOTE: String Splitting & Joining

internal String8_List str8_split_by_chars(Arena *arena, String8 string, u8 *chars, u32 num_chars);
internal String8_List str8_split_by_string_chars(Arena *arena, String8 string, String8 chars);
internal String8 str8_list_join(Arena *arena, String8_List *list, String_Join *opt_join);

////////////////////////////
// NOTE: String Path Helpers

internal String8 str8_chop_last_slash(String8 string);
internal String8 str8_skip_last_slash(String8 string);
internal String8 str8_chop_last_dot(String8 string);
internal String8 str8_skip_last_dot(String8 string);

internal String8_List str8_split_path(Arena *arena, String8 path);
internal String8 str8_list_join_by_style(Arena *arena, String8_List *list, Path_Style style);

/////////////////////////////////////////
// NOTE: UTF-8 & UTF-16 Decoding/Encoding

internal Unicode_Decode utf8_decode(u8 *str, uxx max);
internal Unicode_Decode utf16_decode(u16 *str, uxx max);
internal u32 utf8_encode(u8 *str, u32 codepoint);
internal u32 utf16_encode(u16 *str, u32 codepoint);

///////////////////////////////////
// NOTE: Unicode String Conversions

internal String8 str8_from_str16(Arena *arena, String16 in);
internal String8 str8_from_str32(Arena *arena, String32 in);
internal String16 str16_from_str8(Arena *arena, String8 in);
internal String32 str32_from_str8(Arena *arena, String8 in);

#endif // KRUEGER_BASE_STRING_H
