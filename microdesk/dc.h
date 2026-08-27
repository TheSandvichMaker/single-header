#ifndef DC_HEADER
#define DC_HEADER

// Editor
#if _10X_EDITOR
	#define _WIN32 1
	#define DC_IMPL 1
	#define DC_ARENA_DEBUG 
#endif

#ifndef DC_USE_BACKSLASH_AS_PATH_SEPARATOR_ON_WINDOWS
#define DC_USE_BACKSLASH_AS_PATH_SEPARATOR_ON_WINDOWS 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>

//
// Typedefs
//

#ifndef __cplusplus
	#include <stdbool.h>
	#ifndef alignof
		#define alignof _Alignof
	#endif

	#define dc_thread_local __declspec(thread)
#endif

typedef float    f32;
typedef double   f64;

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef u8       b8;
typedef u16      b16;
typedef u32      b32;
typedef u64      b64;

typedef intptr_t isz;
typedef intptr_t isz;

typedef size_t   usz;

typedef int32_t rune;

//
// Macros
//

// Decorators
#if defined(DC_STATIC)
#define fn      static
#define global  static
#else
#define fn      extern
#define global  extern
#endif

#define local_persist static
#define fn_local      static inline
#define fn_export     extern __declspec(dllexport)

// Meta-macro hackery
#define EXPAND(x) x
#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)
#define PASTE_(a, b) a##b
#define PASTE(a, b) PASTE_(a, b)

#define PAD(n) char PASTE(pad__, __LINE__)[n]

#define ArrayCount(x) ((isz)(sizeof(x) / sizeof((x)[0])))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(x, lo, hi) ((x) < (lo) ? (lo) : (x) > (hi) ? (hi) : (x))

#define KB(x) ((isz)(x) << 10)
#define MB(x) ((isz)(x) << 20)
#define GB(x) ((isz)(x) << 30)
#define TB(x) ((isz)(x) << 40)

#define round_up_pow2(x, y) ((y) > 0 ? (((x) + ((y) - 1)) & (-(isz)y)) : (x))

fn_local u8 *align_pointer(void *ptr, isz align)
{
	return (u8 *)round_up_pow2((uintptr_t)ptr, align);
}

// Linked lists
#define sll_pop(f) f; (f) = (f)->next
#define sll_push(f, n) ((n)->next = (f), (f) = (n))

#define sll_push_back_ex(f, l, n, next) (((l) != NULL ? ((l)->next = (n), (l) = (n)) : ((f) = (n), (l) = (n))), (n)->next = NULL)
#define sll_push_back(f, l, n) sll_push_back_ex(f, l, n, next)

#define dll_push_front_ex(f, l, n, next, prev) ((f) == NULL ? (f) = (l) = (n) : ((f)->prev = (n), (n)->next = (f), (n)->prev = NULL, (f) = (n)))
#define dll_push_back_ex(f, l, n, next, prev)  ((f) == NULL ? (f) = (l) = (n) : ((n)->prev = (l), (n)->next = NULL, (l)->next = (n), (l) = (n)))

#define dll_push_front(f, l, n) dll_push_front_ex(f, l, n, next, prev)
#define dll_push_back(f, l, n) dll_push_back_ex(f, l, n, next, prev)

#define dll_remove_ex(f, l, n, next, prev)                   \
	(                                                        \
	  ((n)->prev != NULL ? (n)->prev->next = (n)->next : 0), \
	  ((n)->next != NULL ? (n)->prev->next = (n)->prev : 0), \
	  ((n) == (f)        ? (f) = (n)->next             : 0), \
	  ((n) == (l)        ? (l) = (n)->prev             : 0)  \
	)

#define dll_remove(f, l, n) dll_remove_ex(f, l, n, next, prev)

#ifdef __cplusplus
#define DC_COMPOUND_LIT(type) type
#else
#define DC_COMPOUND_LIT(type) (type)
#endif

#define dc_sizeof(type) ((isz)sizeof(type))
#define dc_alignof(type) alignof(type)

#define DC_DEFER_LOOP(begin, end) for (i32 PASTE(_i_, __LINE__) = (begin, 0); !PASTE(_i_, __LINE__); PASTE(_i_, __LINE__) += (end, 1))

//
//
//

typedef struct String
{
	char *bytes;
	isz   count;
} String;

typedef struct String_Pair
{
	union
	{
		struct
		{
			String l;
			String r;
		};
		String s[2];
	};
} String_Pair;

typedef struct String16
{
	u16 *bytes; // apologies for the misnomer
	isz  count;
} String16;

//
// Assert
//

// TODO
#include <assert.h>

#define dc_assert(...) assert(__VA_ARGS__)
#define dc_always(x) (assert(x), x)
#define dc_never(x) (assert(!(x)), x)

//
// Memory
//

typedef struct Memory
{
	void *bytes;
	isz   size;
} Memory;

fn_local Memory memory_from_string(String str)
{
	Memory result;
	result.bytes = str.bytes;
	result.size  = str.count;
	return result;
}

fn_local String string_from_memory(Memory mem)
{
	String result;
	result.bytes = (char *)mem.bytes;
	result.count = mem.size;
	return result;
}

// Virtual Memory
fn Memory vm_reserve(void *address, isz size);
fn bool vm_commit(void *address, isz size);
fn Memory vm_alloc(void *address, isz size);
fn void vm_decommit(void *address, isz size);
fn void vm_release(void *address);

// memcpy and memset type stuff
#define copy_bytes(dst, src, len) (memmove(dst, src, (size_t)(len)), dst)
#define copy_array(dst, src, count) copy_bytes(dst, src, (count) * dc_sizeof(src[0]))
#define zero_memory(mem, len) memset(mem, 0, (size_t)(len))
#define zero_array(mem, count) zero_memory(mem, (count) * dc_sizeof((mem)[0]))
#define zero_struct(mem) zero_array(mem, 1)

// Arena
#ifndef DC_ARENA_DEBUG
#define DC_ARENA_DEBUG 0
#endif

#if DC_ARENA_DEBUG
#define DC_ARENA_BREADCRUMB(arena) arena_set_next_debug_info(arena, S(__FILE__), __LINE__)

typedef struct Arena_Debug_Node
{
	struct Arena_Debug_Node *next;
	struct Arena_Debug_Node *prev;
	String tag;
	String file;
	isz line;
	isz index;
	isz offset_to_memory;
	isz allocation_size;
} Arena_Debug_Node;

typedef struct Arena_Debug_State
{
	String current_tag;
	String current_file;
	isz    current_line;
	isz    allocation_count;

	Arena_Debug_Node *first_node;
	Arena_Debug_Node *last_node;
} Arena_Debug_State;

#else
#define DC_ARENA_BREADCRUMB(...) 0
#endif

#define DC_ARENA_CALL(arena, func, ...) (DC_ARENA_BREADCRUMB(arena), func(arena, ## __VA_ARGS__))

#define DC_ARENA_DEFAULT_CAPACITY          GB(8)
#define DC_ARENA_COMMIT_CHUNK_SIZE         KB(256)
#define DC_ARENA_DEFAULT_MIN_COMMIT_CHARGE DC_ARENA_COMMIT_CHUNK_SIZE
#define DC_ARENA_MAX_ALIGN                 64

/* maybe, eventually
typedef enum Arena_Mode
{
	Arena_Mode_virtual,
	Arena_Mode_chunked,
	Arena_Mode_fixed,
} Arena_Mode;
*/

typedef struct Arena_Desc
{
	isz capacity;
} Arena_Desc;

typedef struct Arena_Scope
{
	struct Arena_Scope *next;
	u8    *reset_point;

#if DC_ARENA_DEBUG
	Arena_Debug_State debug_state;
#endif
} Arena_Scope;

typedef struct Arena
{
	struct Arena *parent;
	struct Arena *first_child;
	struct Arena *next;

	u8 *buffer;
	u8 *at;
	u8 *end;
	u8 *committed;
	u8 *zeroed_watermark;

	Arena_Scope *current_scope;

#if DC_ARENA_DEBUG
	Arena_Debug_State debug;
#endif
} Arena;

#if DC_ARENA_DEBUG
fn void arena_set_alloc_tag(Arena *arena, String tag);
fn void arena_set_next_debug_info(Arena *arena, String file, isz line);
#else
#define arena_set_alloc_tag(...)
#define arena_set_next_debug_info(...)
#endif

fn void *arena_alloc_ex_(Arena *arena, isz size, isz align, bool zero_memory, bool do_not_allocate_debug_node);

#define arena_alloc_ex(arena, size, align, zero_memory) DC_ARENA_CALL(arena, arena_alloc_ex_, size, align, zero_memory, false)
#define arena_alloc(arena, size, align) arena_alloc_ex(arena, size, align, true)
#define arena_alloc_nozero(arena, size, align) arena_alloc_ex(arena, size, align, false)
#define arena_alloc_array(arena, count, type) (type *)arena_alloc(arena, dc_sizeof(type) * (count), dc_alignof(type))
#define arena_alloc_array_nozero(arena, count, type) (type *)arena_alloc_nozero(arena, dc_sizeof(type) * (count), dc_alignof(type))
#define arena_alloc_struct(arena, type) arena_alloc_array(arena, 1, type)
#define arena_alloc_struct_nozero(arena, type) arena_alloc_array_nozero(arena, 1, type)

#define arena_copy(arena, src, size) copy_bytes(arena_alloc_nozero(arena, size, 16), src, size)

fn void arena_init(Arena *arena, String name, Arena_Desc const *desc);
fn Arena *arena_make(String name, Arena_Desc const *desc);
fn Arena *arena_make_default(String name);
fn Arena *arena_make_child(Arena *parent, String name, Arena_Desc const *desc);
fn void arena_add_child(Arena *parent, Arena *child);
fn void arena_destroy(Arena *arena); // destroys all children
fn void arena_destroy_nonrecursive(Arena *arena); // does not destroy children
fn Arena *arena_bootstrap_(String name, isz size, isz align, isz offset_to_member, Arena **typecheck);
#define arena_bootstrap(type, arena_member) (type *)arena_bootstrap_(S(#type), dc_sizeof(type), dc_alignof(type), dc_offsetof(type, arena_member), &((type *)(0))->arena_member)

fn isz arena_capacity(Arena const *arena);
fn isz arena_used(Arena const *arena);
fn isz arena_committed(Arena const *arena);
fn isz arena_remaining(Arena const *arena);
fn isz arena_remaining_with_align(Arena const *arena, isz align);
fn void arena_ensure_committed(Arena *arena, isz size, isz align);

typedef u32 Arena_Reset_Flags;
enum
{
	Arena_Reset_Flag_decommit              = (1u << 0), // decommits up to `min_commit_charge`
	Arena_Reset_Flag_decommit_if_low_usage = (1u << 1), // decommits up to `min_commit_charge` if less than half the committed space was used
	Arena_Reset_Flag_reset_children        = (1u << 2),
};

fn void arena_reset_ex(Arena *arena, isz min_commit_charge, Arena_Reset_Flags flags);

 // decommit_if_low_usage with DC_ARENA_DEFAULT_MIN_COMMIT_CHARGE, does not reset children
fn void arena_reset(Arena *arena);

fn bool arena_verify(Arena *arena);

// Temp
fn void arena_scope_begin(Arena *arena);
fn void arena_scope_end(Arena *arena);
fn void arena_scope_abandon(Arena *arena);
fn Arena *arena_get_temp(void);
fn void arena_release_temp(Arena *temp);
fn Arena *arena_get_temp_unscoped(void); // returns the same arena as arena_get_temp most recently returned for this thread, so you are always in the innermost scope
fn void arena_reset_temp_arenas(void);

#define Arena_Scope(arena) DC_DEFER_LOOP(arena_scope_begin(arena), arena_scope_end(arena))
#define Arena_ScopedTemp for (Arena *temp = arena_get_temp(); temp != NULL; (arena_release_temp(temp), temp = NULL))

//
// Strings
//

#define UTF_REPLACEMENT_CHAR 0xFFFDu

#define Sc(text)                         { (char *)("" text), dc_sizeof(text) - 1 }
#define S(text)  DC_COMPOUND_LIT(String) { (char *)("" text), dc_sizeof(text) - 1 }
// Usage: printf("%.*s", Sx(string));
#define Sx(str) (int)(str).count, (str).bytes

#define S16(text) DC_COMPOUND_LIT(String16) { (u16 *)(L"" text), sizeof(L"" text) / sizeof(u16) - 1 }

#define String_Storage(size) struct { isz count; char bytes[size]; }
#define string_from_storage(storage) (DF_STRUCT_LIT(String) { (storage).bytes, (storage.count) })
#define string_into_storage(storage, string) (copy_bytes((storage).bytes, (string).bytes, MIN((isz)ArrayCount((storage).bytes), (string).count)), (storage).count = (string).count)
#define string_storage_size(storage) ((isz)ArrayCount((storage).bytes))

// Single Character
fn bool char_is_whitespace(char c);
fn bool char_is_newline(char c);
fn bool char_is_path_separator(char c);
fn bool char_is_alphabetic(char c);
fn bool char_is_numeric(char c);
fn bool char_is_alphanumeric(char c);
fn char char_to_lower(char c);
fn char char_to_upper(char c);
fn int digit_from_char(char c);
fn i64 digit_from_char_ex(char c, i64 base);

fn bool string_empty(String str);
fn isz cstring_count(char const *str);
fn isz cstring16_count(u16 const *str);
fn String string(char const *chars, isz count);
fn String16 string16(u16 const *chars, isz count);
fn isz string_count_newlines(String string);
fn String string_from_cstring(char const *cstr);
fn String16 string16_from_cstring(u16 const *cstr);
fn String string_from_pointers(char const *start, char const *end);
fn String substring(String str, isz first, isz count);
fn String substring_range(String str, isz first, isz one_past_last);
fn String string_null_terminate(Arena *arena, String str);
fn String16 string16_null_terminate(Arena *arena, String16 str);
fn String string_null_terminate_into_buffer(char *buffer, isz buffer_size, String str);
fn String string_allocate(Arena *arena, isz len);
fn String string_allocate_with_null_terminator(Arena *arena, isz len);
fn String string_copy(Arena *arena, String string);

// Unicode
fn rune utf8_decode(char const **cursor, isz *remaining);
fn rune utf16_decode(u16 const **cursor, isz *remaining);
fn isz utf16_from_utf8_into_buffer(u16 *dst, isz dst_capacity, char const *src, isz src_length);
fn isz utf8_from_utf16_into_buffer(char *dst, isz dst_capacity, u16 const *src, isz src_length);
fn String16 utf16_from_utf8(Arena *arena, String utf8);
fn String utf8_from_utf16(Arena *arena, String16 utf16);

// Formatting
fn String string_to_lower(Arena *arena, String string);
fn String string_to_upper(Arena *arena, String string);
fn void string_to_lower_in_place(String string);
fn void string_to_upper_in_place(String string);
fn String string_format(Arena *arena, char const *fmt, ...);
fn String string_format_va(Arena *arena, char const *fmt, va_list args);
fn String string_format_into_buffer(char *buffer, isz buffer_size, char const *fmt, ...);
fn String string_format_into_buffer_va(char *buffer, isz buffer_size, char const *fmt, va_list args);
fn String string_format_human_readable_bytes(Arena *arena, isz bytes);
fn String string_escapify(Arena *arena, String string);
fn String string_unescapify(Arena *arena, String string);
// fn String string_reindent(Arena *arena, String string, isz depth, char indent_char);

#define Sf(fmt, ...) string_format(arena_get_temp_unscoped(), fmt, ## __VA_ARGS__)

// Compare
typedef uint32_t String_Match_Flags;
enum
{
	SM_insensitive = (1u << 0),
};

fn isz string_compare(String a, String b, String_Match_Flags flags);
fn bool string_match_ex(String a, String b, String_Match_Flags flags);
fn bool string_match(String a, String b);
fn bool string_match_insensitive(String a, String b);
fn bool string_match_prefix(String string, String prefix, String_Match_Flags flags);
fn bool string_match_suffix(String string, String suffix, String_Match_Flags flags);

fn isz string_find_first_char(String string, char c, String_Match_Flags flags);
fn isz string_find_last_char(String string, char c, String_Match_Flags flags);
fn isz string_find_first_non_whitespace(String string);
fn isz string_find_substring(String text, String substring, String_Match_Flags flags);
fn isz string_find_substring_backwards(String text, String substring, String_Match_Flags flags);

/*
Returns the contents of the group PLUS the open/close characters, so that you can distinguish
between an empty group and the group not being present.

	string_find_enclosed_group(S("bla{}"), '{', '}', false) returns "{}",
	string_find_enclosed_group(S("bla"),   '{', '}', false) returns ""

So this is the usage pattern if you care to notice the difference:

	String group = string_find_eclosed_group(...);
	if (!string_empty(group))
	{
		String contents = string_trim(group, 1, 1);
	}

otherwise, you can just write

	String contents = string_trim(string_find_enclosed_group(...), 1, 1);

to unwrap the contents without checking.
*/
fn String string_find_enclosed_group(String string, char open, char close, bool recurse);

fn isz string_calculate_levenshtein_distance(String s, String t, String_Match_Flags flags);

// Paths

#if defined(_WIN32) && DC_USE_BACKSLASH_AS_PATH_SEPARATOR_ON_WINDOWS
#define OS_PATH_SEPARATOR       '\\'
#define OS_OTHER_PATH_SEPARATOR '/'
#else
#define OS_PATH_SEPARATOR       '/'
#define OS_OTHER_PATH_SEPARATOR '\\'
#endif

#if defined(_WIN32)
#define OS_PATH_CASE_SENSITIVE 0
#else
#define OS_PATH_CASE_SENSITIVE 1
#endif

fn void path_normalize_in_place(String *path);
fn String path_normalize(Arena *arena, String path);
fn String path_leaf(String path);
fn String path_directory(String path);
/*
	gets the extension from the path, including the dot
	only the last dot is used to split the extension
	foo.tar.gz -> .gz
*/
fn String path_extension(String path);
/*
	gets the extension from the path, including the dot
	the first dot is used to split the extension
	foo.tar.gz -> .tar.gz
*/
fn String path_long_extension(String path);
fn String path_strip_extension(String path);
fn String path_strip_long_extension(String path);
fn bool path_match_extension(String path, String extension);
fn bool path_match_long_extension(String path, String extension);

// Parsing
fn char *string_start(String string);
fn char *string_end(String string);
fn char string_peek(String string, isz index);
fn String string_trim_left_spaces(String string);
fn String string_trim_right_spaces(String string);
fn String string_trim_spaces(String string);
/*
	Only strips the outermost set of quotes if present.
	In other words, "hello" -> hello, "hello -> "hello, "hello"" -> hello"
*/
fn String string_unquote(String string);
fn String string_trim_blank_lines(String string);
fn String string_skip(String string, isz amount);
fn String string_chop(String string, isz amount);
fn String string_trim(String string, isz from_start, isz from_end);
fn String string_head(String string, isz length);
fn String string_tail(String string, isz length);
fn String_Pair string_split_line(String string);
fn String_Pair string_split_around(String string, String separator);
fn String_Pair string_split_around_char(String string, char c);
/*
	A word for the purposes of this function is just any uninterrupted
	chain of non-whitespace characters
*/
fn String_Pair string_split_word(String string);
fn String_Pair string_split_identifier(String string);
// iterators (modify iter on each call)
fn String string_iter_word(String *iter);
fn String string_iter_line(String *iter);
#define String_EachWord(word, words) String iter = words, word = string_iter_word(&iter); !string_empty(word); word = string_iter_word(&iter)
#define String_EachLine(line, lines) String iter = lines, line = string_iter_line(&iter); !(string_empty(line) && string_empty(iter)); line = string_iter_line(&iter)

typedef struct Parse_Number_Result {
	b32 is_valid;
	i32 overflowed;
	union
	{
		i64 value_i64;
		u64 value_u64;
		i32 value_i32;
		u32 value_u32;
		i16 value_i16;
		u16 value_u16;
		i8  value_i8;
		u8  value_u8;
		f64 value_f64;
		f32 value_f32;
	};
	isz advance;
} Parse_Number_Result;

fn Parse_Number_Result string_parse_i64(String string);
fn Parse_Number_Result string_parse_u64(String string);
fn Parse_Number_Result string_parse_i32(String string);
fn Parse_Number_Result string_parse_u32(String string);
fn Parse_Number_Result string_parse_i16(String string);
fn Parse_Number_Result string_parse_u16(String string);
fn Parse_Number_Result string_parse_i8(String string);
fn Parse_Number_Result string_parse_u8(String string);
fn Parse_Number_Result string_parse_f64(String string);
fn Parse_Number_Result string_parse_f32(String string);

//
// String List
//

typedef struct String_Node
{
    struct String_Node *next;
    struct String_Node *prev;
    String string;
} String_Node;

typedef struct String_List
{
    String_Node *first;
	String_Node *last;
} String_List;

// TODO(daniel): String List functions

//
// String Builder
//

#define DC_STRING_BUILDER_DEFAULT_CHUNK_SIZE 512

typedef struct String_Builder_Chunk
{
	struct String_Builder_Chunk *next;

	char *bytes;
	isz   count;
	isz   capacity;
} String_Builder_Chunk;

typedef struct String_Builder
{
	Arena *arena;
	isz total_count;
	isz line_indent_depth;
	i32 indent_space_count; // if zero, indent with tabs (which you should, you animal)
	bool use_crlf;          // why would you, though?

	String_Builder_Chunk *first_chunk;
	String_Builder_Chunk *last_chunk;
	String_Builder_Chunk *first_free_chunk;
} String_Builder;

typedef u32 String_Reindent_Flags;
enum
{
	String_Reindent_left_justify_preprocessor_defines = (1u << 0),
	String_Reindent_trim_blank_lines                  = (1u << 1),
};

fn void sb_init(String_Builder *sb, Arena *arena);
fn isz sb_appendc(String_Builder *sb, char c);
fn isz sb_appends(String_Builder *sb, String str);
fn isz sb_appendf(String_Builder *sb, char const *fmt, ...);
fn isz sb_appendf_va(String_Builder *sb, char const *fmt, va_list args);
fn isz sb_appendc_n(String_Builder *sb, char c, isz n);
fn isz sb_append_spaces(String_Builder *sb, isz n);
fn isz sb_append_indentation(String_Builder *sb, isz depth);
fn isz sb_append_line_indent(String_Builder *sb);
fn isz sb_line(String_Builder *sb, char const *fmt, ...);
fn isz sb_newline(String_Builder *sb);
fn void sb_push_indent(String_Builder *sb, isz depth);
fn void sb_pop_indent(String_Builder *sb, isz depth);
fn isz sb_append_reindented(String_Builder *sb, String str, isz indent_delta, String_Reindent_Flags flags);
fn void sb_reset(String_Builder *sb);
fn String sb_flatten(Arena *arena, String_Builder *sb);
// binary
fn isz sb_append_bytes(String_Builder *sb, void *bytes, isz n);
fn isz sb_append_bin_i8(String_Builder *sb, i8 v);
fn isz sb_append_bin_i16(String_Builder *sb, i16 v);
fn isz sb_append_bin_i32(String_Builder *sb, i32 v);
fn isz sb_append_bin_i64(String_Builder *sb, i64 v);
fn isz sb_append_bin_u8(String_Builder *sb, u8 v);
fn isz sb_append_bin_u16(String_Builder *sb, u16 v);
fn isz sb_append_bin_u32(String_Builder *sb, u32 v);
fn isz sb_append_bin_u64(String_Builder *sb, u64 v);
fn isz sb_append_bin_f32(String_Builder *sb, f32 v);
fn isz sb_append_bin_f64(String_Builder *sb, f64 v);
// testing
fn bool sb_verify(String_Builder *sb);

//
// File System
//

// TODO(daniel): More file system functionality
fn bool os_write_entire_file(String path, Memory memory);
fn Memory os_read_entire_file(Arena *arena, String path);

#ifdef __cplusplus
}
#endif

#endif

#if defined(DC_IMPL)

#if defined(_WIN32)
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
	#define NONEARFAR

	#if !defined(__cplusplus)
		#define INITGUID
	#endif

	#include <windows.h>
#endif

// Virtual Memory

#if defined(_WIN32)
Memory vm_reserve(void *address, isz size)
{
	Memory result = {0};
	result.bytes = VirtualAlloc(address, size, MEM_RESERVE, PAGE_NOACCESS);

	if (result.bytes)
	{
		result.size = (size + 4095) / 4096;
	}

    return result;
}

bool vm_commit(void *address, isz size)
{
    void *result = VirtualAlloc(address, size, MEM_COMMIT, PAGE_READWRITE);
    return !!result;
}

Memory vm_alloc(void *address, isz size)
{
	Memory result = {0};
    result.bytes = VirtualAlloc(address, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

	if (result.bytes)
	{
		result.size = (size + 4095) / 4096;
	}

    return result;
}

void vm_decommit(void *address, isz size)
{
    VirtualFree(address, size, MEM_DECOMMIT);
}

void vm_release(void *address)
{
    VirtualFree(address, 0, MEM_RELEASE);
}
#else
#error TODO: Other platforms
#endif

// Arena

#if DC_ARENA_DEBUG
void arena_set_alloc_tag(Arena *arena, String tag)
{
	arena->debug.current_tag = string_copy(arena, tag);
}
void arena_set_next_debug_info(Arena *arena, String file, isz line)
{
	arena->debug.current_file = file;
	arena->debug.current_line = line;
}
#endif

void *arena_alloc_ex_(Arena *arena, isz size, isz align, bool zero_memory, bool do_not_allocate_debug_node)
{
	(void)do_not_allocate_debug_node;

	dc_assert(align < DC_ARENA_MAX_ALIGN);

	u8 *result = NULL;

	if (size >= 0)
	{
		arena_ensure_committed(arena, size, align);

		result = (u8 *)align_pointer(arena->at, align);
		dc_assert(result <= arena->end);

		if (zero_memory)
		{
			isz unzeroed_bytes = MIN(arena->zeroed_watermark - result, size);
			if (unzeroed_bytes > 0)
			{
				zero_memory(result, unzeroed_bytes);
			}
		}

		arena->at = result + size;

		if (arena->at > arena->zeroed_watermark)
		{
			arena->zeroed_watermark = arena->at;
		}

#if DC_ARENA_DEBUG
		if (!do_not_allocate_debug_node)
		{
			Arena_Debug_Node *debug = arena_alloc_ex_(arena, dc_sizeof(Arena_Debug_Node), dc_alignof(Arena_Debug_Node), false, true);
			debug->next             = NULL;
			debug->prev             = NULL;
			debug->tag              = arena->debug.current_tag;
			debug->file             = arena->debug.current_file;
			debug->line             = arena->debug.current_line;
			debug->index            = arena->debug.allocation_count++;
			debug->offset_to_memory = result - (u8 *)debug;
			debug->allocation_size  = size;

			sll_push_back(arena->debug.first_node, arena->debug.last_node, debug);
		}

		dc_assert(arena_verify(arena));
#endif
	}

	return result;
}

Arena *arena_make(String name, Arena_Desc const *desc)
{
	(void)name;

	Arena *arena = (Arena *)(vm_reserve(NULL, desc->capacity).bytes);

	bool ok = vm_commit(arena, DC_ARENA_DEFAULT_MIN_COMMIT_CHARGE);
	dc_assert(ok);

	arena->buffer           = (u8 *)align_pointer((arena + 1), DC_ARENA_MAX_ALIGN);
	arena->at               = arena->buffer;
	arena->end              = (u8 *)arena + desc->capacity;
	arena->committed        = (u8 *)arena + DC_ARENA_DEFAULT_MIN_COMMIT_CHARGE;
	arena->zeroed_watermark = arena->buffer;

	return arena;
}

Arena *arena_make_default(String name)
{
	Arena_Desc desc;
	desc.capacity = DC_ARENA_DEFAULT_CAPACITY;

	return arena_make(name, &desc);
}

Arena *arena_make_child(Arena *parent, String name, Arena_Desc const *desc)
{
	Arena *child = arena_make(name, desc);
	arena_add_child(parent, child);

	return child;
}

void arena_add_child(Arena *parent, Arena *child)
{
	child->parent = parent;
	sll_push(parent->first_child, child);
}

void arena_destroy(Arena *arena)
{
	for (Arena *child = arena->first_child; child; child = child->next)
	{
		arena_destroy(child);
	}

	arena_destroy_nonrecursive(arena);
}

void arena_destroy_nonrecursive(Arena *arena)
{
	vm_release(arena);
}

Arena *arena_bootstrap_(String name, isz size, isz align, isz offset_to_member, Arena **typecheck)
{
	(void)typecheck;

	Arena *arena = arena_make_default(name);
	void *result = arena_alloc(arena, size, align);

	// Copying the arena pointer, not the arena
	copy_bytes((u8 *)result + offset_to_member, &arena, sizeof(Arena *));

	return result;
}

isz arena_capacity(Arena const *arena)
{
	return arena->end - arena->buffer;
}

isz arena_used(Arena const *arena)
{
	return arena->at - arena->buffer;
}

isz arena_committed(Arena const *arena)
{
	return arena->committed - (u8 *)arena;
}

isz arena_remaining(Arena const *arena)
{
	return arena->end - arena->at;
}

isz arena_remaining_with_align(Arena const *arena, isz align)
{
	return arena->end - (u8 *)align_pointer(arena->at, align);
}

void arena_ensure_committed(Arena *arena, isz size, isz align)
{
	u8 *at = align_pointer(arena->at, align);
	if (at + size > arena->committed)
	{
		u8 *commit_dst  = align_pointer(at + size, DC_ARENA_COMMIT_CHUNK_SIZE);
		isz commit_size = commit_dst - arena->committed;

		bool ok = vm_commit(arena->committed, commit_size);
		dc_assert(ok);

		arena->committed = commit_dst;
	}
}

void arena_reset_ex(Arena *arena, isz min_commit_charge, Arena_Reset_Flags flags)
{
	if (flags & Arena_Reset_Flag_reset_children)
	{
		for (Arena *child = arena->first_child; child; child = child->next)
		{
			arena_reset_ex(arena, min_commit_charge, flags);
		}
	}

	arena->at = arena->buffer;

	b32 should_decommit = (flags & Arena_Reset_Flag_decommit);

	if (flags & Arena_Reset_Flag_decommit_if_low_usage)
	{
		if (arena_used(arena) * 2 < arena_committed(arena))
		{
			should_decommit = true;
		}
	}

	if (should_decommit && arena_committed(arena) > min_commit_charge)
	{
		u8 *decommit_from = (u8 *)arena + min_commit_charge;
		isz decommit_size = arena->committed - decommit_from;
		vm_decommit(decommit_from, decommit_size);

		arena->committed = decommit_from;
	}

#if DC_ARENA_DEBUG
	arena->debug.current_tag      = S("");
	arena->debug.current_file     = S("");
	arena->debug.current_line     = 1;
	arena->debug.allocation_count = 0;
	arena->debug.first_node       = NULL;
	arena->debug.last_node        = NULL;
#endif
}

void arena_reset(Arena *arena)
{
	arena_reset_ex(arena, DC_ARENA_DEFAULT_MIN_COMMIT_CHARGE, Arena_Reset_Flag_decommit_if_low_usage);
}

bool arena_verify(Arena *arena)
{
	(void)arena;
	// TODO
	return true;
}

// Temp

void arena_scope_begin(Arena *arena)
{
	u8 *reset_point = arena->at;

#if DC_ARENA_DEBUG
	Arena_Debug_State debug_state = arena->debug;
#endif

	Arena_Scope *scope = arena_alloc_struct_nozero(arena, Arena_Scope);

	scope->reset_point = reset_point;

#if DC_ARENA_DEBUG
	scope->debug_state = debug_state;
#endif

	sll_push(arena->current_scope, scope);
}

void arena_scope_end(Arena *arena)
{
	if (dc_always(arena->current_scope != NULL))
	{
		Arena_Scope *scope = sll_pop(arena->current_scope);
		arena->at = scope->reset_point;
#if DC_ARENA_DEBUG
		arena->debug = scope->debug_state;
#endif
	}
}

void arena_scope_abandon(Arena *arena)
{
	if (dc_always(arena->current_scope != NULL))
	{
		sll_pop(arena->current_scope);
		// And then do nothing with it :)
	}
}

typedef struct Thread_Context
{
	Arena *temp_arenas[2];
	isz    temp_arena_index;
} Thread_Context;

dc_thread_local Thread_Context tctx;

Arena *arena_get_temp(void)
{
	Arena *arena = tctx.temp_arenas[tctx.temp_arena_index];
	tctx.temp_arena_index = 1 - tctx.temp_arena_index;

	if (arena == NULL)
	{
		// TODO: thread id
		char buf[64];
		String name = string_format_into_buffer(buf, sizeof(buf), "tctx(-1).temp[%zd]", 1 - tctx.temp_arena_index);

		arena = arena_make_default(name);
		tctx.temp_arenas[1 - tctx.temp_arena_index] = arena;
	}

	arena_scope_begin(arena);
	return arena;
}

void arena_release_temp(Arena *temp)
{
	dc_assert(temp == arena_get_temp_unscoped());

	arena_scope_end(temp);
	tctx.temp_arena_index = 1 - tctx.temp_arena_index;
}

Arena *arena_get_temp_unscoped(void)
{
	return tctx.temp_arenas[1 - tctx.temp_arena_index];
}

void arena_reset_temp_arenas(void)
{
	arena_reset(tctx.temp_arenas[0]);
	arena_reset(tctx.temp_arenas[1]);
}

//
// Strings
//

bool char_is_whitespace(char c)
{
    return c == ' '  || 
           c == '\t' ||
           c == '\n' ||
           c == '\r';
}

bool char_is_newline(char c)
{
    return c == '\n' ||
           c == '\r';
}

bool char_is_path_separator(char c)
{
    return c == '/' ||
           c == '\\';
}

bool char_is_alphabetic(char c)
{
    return ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z'));
}

bool char_is_numeric(char c)
{
    return (c >= '0' && c <= '9');
}

bool char_is_numeric_or_hex(char c)
{
	return char_is_numeric(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

bool char_is_alphanumeric(char c)
{
	bool alpha = char_is_alphabetic(c);
	bool num   = char_is_numeric(c);
	return alpha|num;
}

char char_to_lower(char c)
{
	char result = c;
	if (result >= 'A' && result <= 'Z')
	{
		result += ('a' - 'A');
	}
	return result;
}

char char_to_upper(char c)
{
	char result = c;
	if (result >= 'a' && result <= 'z')
	{
		result += ('A' - 'a');
	}
	return result;
}

int digit_from_char(char c)
{
	if (c >= '0' && c <= '9')
	{
		return c - '0';
	}
	else
	{
		return -1;
	}
}

global i8 digit_from_char_table[] = {
	// [null-/]
	-1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1,
	// [0-9]
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	// [:-@]
	-1, -1, -1, -1, -1, -1, -1,
	// [A-F]
	10, 11, 12, 13, 14, 15,
	// [G-`]
	-1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	// [a-f]
	10, 11, 12, 13, 14, 15,
	// [g-del]
	-1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1,
};

i64 digit_from_char_ex(char c, i64 base)
{
	i32 result = digit_from_char_table[c];

	if (result >= base)
	{
		result = -1;
	}

	return result;
}

bool string_empty(String str)
{
	return str.bytes == NULL || str.count <= 0;
}

isz cstring_count(char const *str)
{
	return (isz)strlen(str);
}

isz cstring16_count(u16 const *str)
{
	isz result = 0;
	for (u16 const *c = str; *c; c += 1, result += 1);
	return result;
}

String string(char const *chars, isz count)
{
	String result = { (char *)chars, count };
	return result;
}

String16 string16(u16 const *chars, isz count)
{
	String16 result = { (u16 *)chars, count };
	return result;
}

isz string_count_newlines(String string)
{
	isz result = 0;
	for (isz i = 0; i < string.count; i += 1)
	{
		if (string.bytes[i] == '\n') result += 1;
	}
	return result;
}

String string_from_cstring(char const *cstr)
{
	return DC_COMPOUND_LIT(String){ (char *)cstr, cstring_count(cstr) };
}

String16 string16_from_cstring(u16 const *cstr)
{
	return DC_COMPOUND_LIT(String16){ (u16 *)cstr, cstring16_count(cstr) };
}

String string_from_pointers(char const *start, char const *end)
{
	return DC_COMPOUND_LIT(String){ (char *)start, (isz)(end - start) };
}

String substring(String str, isz first, isz count)
{
	first = CLAMP(first, 0, str.count);
	count = CLAMP(count, 0, str.count - first);
	return DC_COMPOUND_LIT(String){ str.bytes + first, count };
}

String substring_range(String str, isz first, isz one_past_last)
{
	first = CLAMP(first, 0, str.count);
	one_past_last = CLAMP(one_past_last, first, str.count);
	return DC_COMPOUND_LIT(String){ str.bytes + first, one_past_last - first };
}

String string_null_terminate(Arena *arena, String str)
{
	String result;
	result.bytes = arena_alloc_array_nozero(arena, str.count + 1, char);
	result.count = str.count;

	copy_array(result.bytes, str.bytes, result.count);
	result.bytes[result.count] = '\0';

	return result;
}

String16 string16_null_terminate(Arena *arena, String16 str)
{
	String16 result;
	result.bytes = arena_alloc_array_nozero(arena, str.count + 1, u16);
	result.count = str.count;

	copy_array(result.bytes, str.bytes, result.count);
	result.bytes[result.count] = '\0';

	return result;
}

String string_null_terminate_into_buffer(char *buffer, isz buffer_size, String str)
{
	isz copy_size = MIN(buffer_size - 1, str.count);
	copy_array(buffer, str.bytes, copy_size);
	buffer[copy_size] = '\0';
	return DC_COMPOUND_LIT(String){ buffer, copy_size };
}

rune utf8_decode(char const **cursor, isz *remaining)
{
	local_persist rune const min_code_point[4] = { 0, 0x80, 0x800, 0x10000 };

	u8 const *s = (u8 const *)*cursor;
	isz available = *remaining;
	u8 b0 = s[0];
	rune code_point;
	isz continuation_count;
	isz i;

	if (b0 < 0x80)
	{
		*cursor += 1;
		*remaining -= 1;
		return b0;
	}
	else if (b0 >= 0xC2 && b0 <= 0xDF)
	{
		code_point = b0 & 0x1Fu;
		continuation_count = 1;
	}
	else if (b0 >= 0xE0 && b0 <= 0xEF)
	{
		code_point = b0 & 0x0Fu;
		continuation_count = 2;
	}
	else if (b0 >= 0xF0 && b0 <= 0xF4)
	{
		code_point = b0 & 0x07u;
		continuation_count = 3;
	}
	else
	{
		// Stray continuation byte, or a lead byte that can only start an overlong (0xC0/0xC1) or
		// out-of-range (0xF5+) sequence.
		*cursor += 1;
		*remaining -= 1;
		return UTF_REPLACEMENT_CHAR;
	}

	for (i = 1; i <= continuation_count; i += 1)
	{
		u8 c;

		if (i >= available || ((c = s[i]) & 0xC0u) != 0x80u)
		{
			// The sequence is cut short, either by the end of the source or by a byte that is not
			// a continuation. Consume only the bytes that were actually part of it - never the byte
			// that ended it, it may be the start of a valid sequence, or the terminator.
			*cursor += i;
			*remaining -= (size_t)i;
			return UTF_REPLACEMENT_CHAR;
		}

		code_point = (code_point << 6) | (c & 0x3Fu);
	}

	*cursor += continuation_count + 1;
	*remaining -= continuation_count + 1;

	if (code_point < min_code_point[continuation_count] || code_point > 0x10FFFFu || (code_point >= 0xD800u && code_point <= 0xDFFFu))
	{
		return UTF_REPLACEMENT_CHAR;
	}

	return code_point;
}

rune utf16_decode(u16 const **cursor, isz *remaining)
{
	u16 const *s = *cursor;
	rune unit = s[0];

	if (unit >= 0xD800u && unit <= 0xDBFFu)
	{
		if (*remaining >= 2)
		{
			rune low = s[1];

			if (low >= 0xDC00u && low <= 0xDFFFu)
			{
				*cursor += 2;
				*remaining -= 2;
				return 0x10000u + ((unit - 0xD800u) << 10) + (low - 0xDC00u);
			}
		}

		*cursor += 1;
		*remaining -= 1;
		return UTF_REPLACEMENT_CHAR; // Unpaired high surrogate, or one cut off by the source end.
	}

	*cursor += 1;
	*remaining -= 1;

	if (unit >= 0xDC00u && unit <= 0xDFFFu)
	{
		return UTF_REPLACEMENT_CHAR; // Unpaired low surrogate.
	}

	return unit;
}

isz utf16_from_utf8_into_buffer(u16 *dst, isz dst_capacity, char const *src, isz src_length)
{
	isz required = 1; // The null terminator.
	isz written = 0;
	bool truncated = (dst == NULL || dst_capacity == 0);
	char const *cursor = src;
	isz remaining = (src != NULL) ? src_length : 0;

	while (remaining > 0 && *cursor != '\0')
	{
		rune code_point = utf8_decode(&cursor, &remaining);
		u16 units[2];
		isz count;

		if (code_point < 0x10000u)
		{
			units[0] = (u16)code_point;
			count = 1;
		}
		else
		{
			code_point -= 0x10000u;
			units[0] = (u16)(0xD800u + (code_point >> 10));
			units[1] = (u16)(0xDC00u + (code_point & 0x3FFu));
			count = 2;
		}

		required += count;

		// Write only while the whole code point *and* the terminator still fit. Once anything has
		// been dropped, stop writing entirely rather than emitting a later character out of order.
		if (!truncated && written + count + 1 <= dst_capacity)
		{
			dst[written] = units[0];

			if (count == 2)
			{
				dst[written + 1] = units[1];
			}

			written += count;
		}
		else
		{
			truncated = 1;
		}
	}

	if (dst != NULL && dst_capacity > 0)
	{
		dst[written] = 0;
	}

	return required;
}

isz utf8_from_utf16_into_buffer(char *dst, isz dst_capacity, u16 const *src, isz src_length)
{
	isz required = 1; // The null terminator.
	isz written = 0;
	bool truncated = (dst == NULL || dst_capacity == 0);
	u16 const *cursor = src;
	isz remaining = (src != NULL) ? src_length : 0;

	while (remaining > 0 && *cursor != 0)
	{
		rune code_point = utf16_decode(&cursor, &remaining);
		char bytes[4];
		isz count;
		isz i;

		if (code_point < 0x80u)
		{
			bytes[0] = (char)code_point;
			count = 1;
		}
		else if (code_point < 0x800u)
		{
			bytes[0] = (char)(0xC0u | (code_point >> 6));
			bytes[1] = (char)(0x80u | (code_point & 0x3Fu));
			count = 2;
		}
		else if (code_point < 0x10000u)
		{
			bytes[0] = (char)(0xE0u | (code_point >> 12));
			bytes[1] = (char)(0x80u | ((code_point >> 6) & 0x3Fu));
			bytes[2] = (char)(0x80u | (code_point & 0x3Fu));
			count = 3;
		}
		else
		{
			bytes[0] = (char)(0xF0u | (code_point >> 18));
			bytes[1] = (char)(0x80u | ((code_point >> 12) & 0x3Fu));
			bytes[2] = (char)(0x80u | ((code_point >> 6) & 0x3Fu));
			bytes[3] = (char)(0x80u | (code_point & 0x3Fu));
			count = 4;
		}

		required += count;

		if (!truncated && written + count + 1 <= dst_capacity)
		{
			for (i = 0; i < count; i += 1)
			{
				dst[written + i] = bytes[i];
			}

			written += count;
		}
		else
		{
			truncated = 1;
		}
	}

	if (dst != NULL && dst_capacity > 0)
	{
		dst[written] = 0;
	}

	return required;
}

String16 utf16_from_utf8(Arena *arena, String utf8)
{
	isz required = utf16_from_utf8_into_buffer(NULL, 0, utf8.bytes, utf8.count);

	String16 result;
	result.bytes = arena_alloc_array_nozero(arena, required + 1, u16);
	result.count = required;

	utf16_from_utf8_into_buffer(result.bytes, result.count, utf8.bytes, utf8.count);

	return result;
}

String utf8_from_utf16(Arena *arena, String16 utf16)
{
	isz required = utf8_from_utf16_into_buffer(NULL, 0, utf16.bytes, utf16.count);

	String result = string_allocate_with_null_terminator(arena, required - 1);
	utf8_from_utf16_into_buffer(result.bytes, required, utf16.bytes, utf16.count);

	return result;
}

String string_allocate(Arena *arena, isz len)
{
	len = MAX(0, len);

	String result;
	result.bytes = arena_alloc_array_nozero(arena, len, char);
	result.count = len;
	return result;
}

String string_allocate_with_null_terminator(Arena *arena, isz len)
{
	len = MAX(0, len);

	String result;
	result.bytes = arena_alloc_array_nozero(arena, len + 1, char);
	result.count = len;
	return result;
}

String string_copy(Arena *arena, String string)
{
	return string_null_terminate(arena, string);
}

// Formatting
String string_to_lower(Arena *arena, String string)
{
	String result = string_allocate(arena, string.count);
	for (isz i = 0; i < string.count; i += 1)
	{
		result.bytes[i] = char_to_lower(string.bytes[i]);
	}
	return result;
}

String string_to_upper(Arena *arena, String string)
{
	String result = string_allocate(arena, string.count);
	for (isz i = 0; i < string.count; i += 1)
	{
		result.bytes[i] = char_to_upper(string.bytes[i]);
	}
	return result;
}

void string_to_lower_in_place(String string)
{
	for (isz i = 0; i < string.count; i += 1)
	{
		string.bytes[i] = char_to_lower(string.bytes[i]);
	}
}

void string_to_upper_in_place(String string)
{
	for (isz i = 0; i < string.count; i += 1)
	{
		string.bytes[i] = char_to_upper(string.bytes[i]);
	}
}

/* -----------------------------------------------------------------------------------------------------
   DC: What follows is an embedded copy of stb_sprintf with a modification to handle counted string
   types. The header is here, the implementation sits at the bottom of the file.
   -----------------------------------------------------------------------------------------------------*/

#define STB_SPRINTF_STATIC

// stb_sprintf - v1.10 - public domain snprintf() implementation
// originally by Jeff Roberts / RAD Game Tools, 2015/10/20
// http://github.com/nothings/stb
//
// allowed types:  sc uidBboXx p AaGgEef n
// lengths      :  hh h ll j z t I64 I32 I
//
// Contributors:
//    Fabian "ryg" Giesen (reformatting)
//    github:aganm (attribute format)
//
// Contributors (bugfixes):
//    github:d26435
//    github:trex78
//    github:account-login
//    Jari Komppa (SI suffixes)
//    Rohit Nirmal
//    Marcin Wojdyr
//    Leonard Ritter
//    Stefano Zanotti
//    Adam Allison
//    Arvid Gerstmann
//    Markus Kolb
//
// LICENSE:
//
//   See end of file for license information.

#if defined(__clang__)
 #if defined(__has_feature) && defined(__has_attribute)
  #if __has_feature(address_sanitizer)
   #if __has_attribute(__no_sanitize__)
    #define STBSP__ASAN __attribute__((__no_sanitize__("address")))
   #elif __has_attribute(__no_sanitize_address__)
    #define STBSP__ASAN __attribute__((__no_sanitize_address__))
   #elif __has_attribute(__no_address_safety_analysis__)
    #define STBSP__ASAN __attribute__((__no_address_safety_analysis__))
   #endif
  #endif
 #endif
#elif defined(__GNUC__) && (__GNUC__ >= 5 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8))
 #if defined(__SANITIZE_ADDRESS__) && __SANITIZE_ADDRESS__
  #define STBSP__ASAN __attribute__((__no_sanitize_address__))
 #endif
#elif defined(_MSC_VER)
 #if defined(__SANITIZE_ADDRESS__) && __SANITIZE_ADDRESS__
  #define STBSP__ASAN __declspec(no_sanitize_address)
 #endif
#endif

#ifndef STBSP__ASAN
#define STBSP__ASAN
#endif

#ifdef STB_SPRINTF_STATIC
#define STBSP__PUBLICDEC static STBSP__ASAN
#define STBSP__PUBLICDEF static STBSP__ASAN
#else
#ifdef __cplusplus
#define STBSP__PUBLICDEC extern "C" STBSP__ASAN
#define STBSP__PUBLICDEF extern "C" STBSP__ASAN
#else
#define STBSP__PUBLICDEC extern STBSP__ASAN
#define STBSP__PUBLICDEF STBSP__ASAN
#endif
#endif

#if defined(__has_attribute)
 #if __has_attribute(format)
   #define STBSP__ATTRIBUTE_FORMAT(fmt,va) __attribute__((format(printf,fmt,va)))
 #endif
#endif

#ifndef STBSP__ATTRIBUTE_FORMAT
#define STBSP__ATTRIBUTE_FORMAT(fmt,va)
#endif

#ifdef _MSC_VER
#define STBSP__NOTUSED(v)  (void)(v)
#else
#define STBSP__NOTUSED(v)  (void)sizeof(v)
#endif

#ifndef STB_SPRINTF_MIN
#define STB_SPRINTF_MIN 512 // how many characters per callback
#endif
typedef char *STBSP_SPRINTFCB(const char *buf, void *user, int len);

#ifndef STB_SPRINTF_DECORATE
#define STB_SPRINTF_DECORATE(name) stbsp_##name // define this before including if you want to change the names
#endif

STBSP__PUBLICDEC int STB_SPRINTF_DECORATE(vsprintf)(char *buf, char const *fmt, va_list va);
STBSP__PUBLICDEC int STB_SPRINTF_DECORATE(vsnprintf)(char *buf, int count, char const *fmt, va_list va);
STBSP__PUBLICDEC int STB_SPRINTF_DECORATE(sprintf)(char *buf, char const *fmt, ...) STBSP__ATTRIBUTE_FORMAT(2,3);
STBSP__PUBLICDEC int STB_SPRINTF_DECORATE(snprintf)(char *buf, int count, char const *fmt, ...) STBSP__ATTRIBUTE_FORMAT(3,4);

STBSP__PUBLICDEC int STB_SPRINTF_DECORATE(vsprintfcb)(STBSP_SPRINTFCB *callback, void *user, char *buf, char const *fmt, va_list va);
STBSP__PUBLICDEC void STB_SPRINTF_DECORATE(set_separators)(char comma, char period);

/* -----------------------------------------------------------------------------------------------------
   DC: End of third-party code
   -----------------------------------------------------------------------------------------------------*/

String string_format(Arena *arena, char const *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	String result = string_format_va(arena, fmt, args);

	va_end(args);

	return result;
}

String string_format_va(Arena *arena, char const *fmt, va_list args)
{
	va_list args2;
	va_copy(args2, args);

	isz required = stbsp_vsnprintf(NULL, 0, fmt, args2);

	va_end(args2);

	String result;
	result.bytes = arena_alloc_array_nozero(arena, required + 1, char);
	result.count = required;

	stbsp_vsnprintf(result.bytes, (int)(result.count + 1), fmt, args);

	return result;
}

String string_format_into_buffer(char *buffer, isz buffer_size, char const *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	String result = string_format_into_buffer_va(buffer, buffer_size, fmt, args);

	va_end(args);

	return result;
}

String string_format_into_buffer_va(char *buffer, isz buffer_size, char const *fmt, va_list args)
{
	int count = stbsp_vsnprintf(buffer, (int)buffer_size, fmt, args);

	String result;
	result.bytes = buffer;
	result.count = count;

	return result;
}

String string_format_human_readable_bytes(Arena *arena, isz bytes)
{
	isz log = 0;
	for (isz x = bytes / 1024; x != 0; x /= 1024) log += 1;

	String result;
	switch (log)
	{
		case  0: result = string_format(arena, "%zdB", bytes); break;
		case  1: result = string_format(arena, "%.02fKiB", (double)bytes / 1024.0); break;
		case  2: result = string_format(arena, "%.02fMiB", (double)bytes / (1024.0 * 1024.0)); break;
		case  3: result = string_format(arena, "%.02fGiB", (double)bytes / (1024.0 * 1024.0 * 1024.0)); break;
		default: result = string_format(arena, "%.02fTiB", (double)bytes / (1024.0 * 1024.0 * 1024.0 * 1024.0)); break;
	}

	return result;
}

String string_escapify(Arena *arena, String string)
{
	String result = string_allocate(arena, 2 * string.count);

	isz at = 0;
	for (isz i = 0; i < string.count; i += 1)
	{
		switch (string.bytes[i])
		{
			case '\n': result.bytes[at++] = '\\'; result.bytes[at++] = '\n'; break;
			case '\r': result.bytes[at++] = '\\'; result.bytes[at++] = '\r'; break;
			case '\t': result.bytes[at++] = '\\'; result.bytes[at++] = '\t'; break;
			case '\v': result.bytes[at++] = '\\'; result.bytes[at++] = '\v'; break;
			case  '"': result.bytes[at++] = '\\'; result.bytes[at++] =  '"'; break;
			case '\\': result.bytes[at++] = '\\'; result.bytes[at++] = '\\'; break;
			default: result.bytes[at++] = string.bytes[i];
		}
	}
	result.count = at;

	return result;
}

String string_unescapify(Arena *arena, String string)
{
	String result = string_allocate(arena, string.count);

	isz at = 0;
	for (isz i = 0; i < string.count; i += 1)
	{
		if (i + 1 < string.count && string.bytes[i] == '\\')
		{
			i += 1;
			switch (string.bytes[i])
			{
				case  'n': result.bytes[at++] = '\n'; break;
				case  'r': result.bytes[at++] = '\r'; break;
				case  't': result.bytes[at++] = '\t'; break;
				case  'v': result.bytes[at++] = '\v'; break;
				case  '"': result.bytes[at++] =  '"'; break;
				case '\\': result.bytes[at++] = '\\'; break;
				default:
				{
					// TODO(daniel): Return/report information about invalid escapes?
					result.bytes[at++] = '\\';
					result.bytes[at++] = string.bytes[i];
				} break;
			}
		}
		else
		{
			result.bytes[at++] = string.bytes[i];
		}
	}
	result.count = at;

	return result;
}

isz string_compare(String a, String b, String_Match_Flags flags)
{
	isz diff = 0;
	for (isz i = 0; i < MIN(a.count, b.count); i += 1)
	{
		char ch_a = a.bytes[i];
		char ch_b = b.bytes[i];
		if (flags & SM_insensitive)
		{
			ch_a = char_to_lower(ch_a);
			ch_b = char_to_lower(ch_b);
		}
		diff = ch_a - ch_b;
		if (diff != 0) break;
	}
	if (diff == 0 && a.count != b.count) diff = a.count < b.count ? -1 : 1;
	return diff;
}

bool string_match_ex(String a, String b, String_Match_Flags flags)
{
	if (a.count != b.count) return false;
	for (isz i = 0; i < a.count; i += 1)
	{
		char ch_a = a.bytes[i];
		char ch_b = b.bytes[i];
		if (flags & SM_insensitive)
		{
			ch_a = char_to_lower(ch_a);
			ch_b = char_to_lower(ch_b);
		}
		if (ch_a != ch_b) return false;
	}
	return true;
}

// TODO(daniel): Computer Enhance homework! Is this faster than the naive loop?
bool string_match(String a, String b)
{
	if (a.count != b.count) return false;
	isz oct_count = (a.count >> 3);
	isz single_count = a.count - (oct_count << 3);
	u64 *oct_a = (u64 *)a.bytes;
	u64 *oct_b = (u64 *)b.bytes;
	for (isz i = 0; i < oct_count; i += 1)
	{
		if (oct_a[i] != oct_b[i]) return false;
	}
	for (isz i = (oct_count << 3); i < (oct_count << 3) + single_count; i += 1)
	{
		if (a.bytes[i] != b.bytes[i]) return false;
	}
	return true;
}

bool string_match_insensitive(String a, String b)
{
	return string_match_ex(a, b, SM_insensitive);
}

bool string_match_prefix(String string, String prefix, String_Match_Flags flags)
{
	if (prefix.count > string.count) return false;
	return string_match_ex(substring(string, 0, prefix.count), prefix, flags);
}

bool string_match_suffix(String string, String suffix, String_Match_Flags flags)
{
	if (suffix.count > string.count) return false;
	return string_match_ex(substring(string, string.count - suffix.count, suffix.count), suffix, flags);
}

isz string_find_first_char(String string, char c, String_Match_Flags flags)
{
	isz i = 0;
	if (flags & SM_insensitive)
	{
		c = char_to_lower(c);
		for (; i < (string.count & ~3); i += 4)
		{
			if (char_to_lower(string.bytes[i + 0]) == c) return i + 0;
			if (char_to_lower(string.bytes[i + 1]) == c) return i + 1;
			if (char_to_lower(string.bytes[i + 2]) == c) return i + 2;
			if (char_to_lower(string.bytes[i + 3]) == c) return i + 3;
		}
		for (; i < string.count; i += 1)
		{
			if (char_to_lower(string.bytes[i]) == c) return i;
		}
	}
	else
	{
		for (; i < (string.count & ~3); i += 4)
		{
			if (string.bytes[i + 0] == c) return i + 0;
			if (string.bytes[i + 1] == c) return i + 1;
			if (string.bytes[i + 2] == c) return i + 2;
			if (string.bytes[i + 3] == c) return i + 3;
		}
		for (; i < string.count; i += 1)
		{
			if (string.bytes[i] == c) return i;
		}
	}
	return string.count;
}

isz string_find_first_non_whitespace(String string)
{
	for (isz i = 0; i < string.count; i += 1)
	{
		if (!char_is_whitespace(string.bytes[i]))
		{
			return i;
		}
	}
	return string.count;
}

isz string_find_last_char(String string, char c, String_Match_Flags flags)
{
	if (flags & SM_insensitive)
	{
		c = char_to_lower(c);
		for (isz i = string.count - 1; i >= 0; i -= 1)
		{
			if (char_to_lower(string.bytes[i]) == c) return i;
		}
	}
	else
	{
		for (isz i = string.count - 1; i >= 0; i -= 1)
		{
			if (string.bytes[i] == c) return i;
		}
	}
	return string.count;
}

isz string_find_substring(String text, String pattern, String_Match_Flags flags)
{
	// TODO(daniel): cool fast version
	if (pattern.count > text.count) return text.count;

	String window = substring(text, 0, pattern.count);
	for (isz i = 0; i < text.count - pattern.count; i += 1)
	{
		if (string_match_ex(window, pattern, flags))
		{
			return i;
		}
		window.bytes += 1;
	}
	return text.count;
}

isz string_find_substring_backwards(String text, String pattern, String_Match_Flags flags)
{
	// TODO(daniel): cool fast version
	if (pattern.count > text.count) return text.count;

	String window = substring(text, text.count - pattern.count, pattern.count);
	for (isz i = text.count - pattern.count; i >= 0; i -= 1)
	{
		if (string_match_ex(window, pattern, flags))
		{
			return i;
		}
		window.bytes -= 1;
	}
	return text.count;
}

fn String string_find_enclosed_group(String string, char open, char close, bool recurse)
{
	String result = {0};

	isz i = string_find_first_char(string, open, 0);
	if (i >= string.count - 1) return result;

	char *start = &string.bytes[i];

    i += 1;

	isz depth = 1;
	while (i < string.count && depth > 0)
	{
		if (string.bytes[i] == open)
		{
			if (recurse) depth += 1; else start = &string.bytes[i];
		}
		else if (string.bytes[i] == close)
		{
			depth -= 1;
		}
        i += 1;
		if (depth == 0) break;
	}

	char *end  = &string.bytes[i];

	if (depth == 0)
	{
		result.bytes = start;
		result.count = end - start;
	}

	return result;
}

isz string_calculate_levenshtein_distance(String s, String t, String_Match_Flags flags)
{
	// TODO(daniel): Improve this naive implementation
	if (s.count * t.count > (1ll << 16)) return -1;

	i32 n = (i32)s.count;
	i32 m = (i32)t.count;
	i32 k = n + 1;

	i32 result = -1;

	Arena_ScopedTemp {
		i32 *matrix = arena_alloc_array(temp, (n + 1) * (m + 1), i32);

		for (i32 i = 1; i <= n; i += 1) matrix[i    ] = i;
		for (i32 i = 1; i <= m; i += 1) matrix[i * k] = i;

		for (i32 i = 1; i <= n; i += 1)
		{
			char si = s.bytes[i - 1];

			if (flags & SM_insensitive) si = char_to_lower(si);

			for (i32 j = 1; j <= m; j += 1)
			{
				char tj = t.bytes[j - 1];

				if (flags & SM_insensitive) tj = char_to_lower(tj);

				i32 cost = (si == tj ? 0 : 1);

				i32 a = matrix[(i - 1) + (j    ) * k] + 1;
				i32 b = matrix[(i    ) + (j - 1) * k] + 1;
				i32 c = matrix[(i - 1) + (j - 1) * k] + cost;

				matrix[i + j*k] = MIN(a, MIN(b, c));
			}
		}

		result = matrix[n + m*k];
	}

	return result;
}

// TODO(daniel): Normalize more (look at Odin's path library)
void path_normalize_in_place(String *path)
{
	isz at = 0;
	bool last_was_path_separator = false;
	for (isz i = 0; i < path->count; i += 1)
	{
		if (char_is_path_separator(path->bytes[i]))
		{
			if (!last_was_path_separator) path->bytes[at++] = OS_PATH_SEPARATOR;
			last_was_path_separator = true;
		}
		else
		{
			path->bytes[at++] = path->bytes[i];
			last_was_path_separator = false;
		}
	}
	path->count = at;
}

String path_normalize(Arena *arena, String path)
{
	String result = string_allocate(arena, path.count);

	isz at = 0;
	bool last_was_path_separator = false;
	for (isz i = 0; i < path.count; i += 1)
	{
		if (char_is_path_separator(path.bytes[i]))
		{
			if (!last_was_path_separator) result.bytes[at++] = OS_PATH_SEPARATOR;
			last_was_path_separator = true;
		}
		else
		{
			result.bytes[at++] = path.bytes[i];
			last_was_path_separator = false;
		}
	}
	result.count = at;

	return result;
}

String path_leaf(String path)
{
	isz j = 0;
	for (isz i = 0; i < path.count; i += 1)
	{
		if (char_is_path_separator(path.bytes[i]))
		{
			j = i + 1;
		}
	}
	return substring(path, j, path.count);
}

String path_directory(String path)
{
	isz j = 0;
	for (isz i = 0; i < path.count; i += 1)
	{
		if (char_is_path_separator(path.bytes[i]))
		{
			j = i;
		}
	}
	return substring(path, 0, j);
}

String path_extension(String path)
{
	isz i = string_find_last_char(path, '.', 0);
	return substring(path, i, path.count);
}

String path_long_extension(String path)
{
	isz i = string_find_first_char(path, '.', 0);
	return substring(path, i, path.count);
}

String path_strip_extension(String path)
{
	isz i = string_find_last_char(path, '.', 0);
	return substring(path, 0, i);
}

String path_strip_long_extension(String path)
{
	isz i = string_find_first_char(path, '.', 0);
	return substring(path, 0, i);
}

bool path_match_extension(String path, String extension)
{
	String ext = path_extension(path);
#if OS_PATH_CASE_SENSITIVE
	return string_match(ext, extension);
#else
	return string_match_insensitive(ext, extension);
#endif
}

bool path_match_long_extension(String path, String extension)
{
	String ext = path_long_extension(path);
#if OS_PATH_CASE_SENSITIVE
	return string_match(ext, extension);
#else
	return string_match_insensitive(ext, extension);
#endif
}

char *string_start(String string)
{
	return string.bytes;
}

char *string_end(String string)
{
	return &string.bytes[string.count];
}

char string_peek(String string, isz index)
{
	if (index < 0) index = string.count - index;
	if (index < string.count)
	{
		return string.bytes[index];
	}
	return 0;
}

String string_trim_left_spaces(String string)
{
	while (char_is_whitespace(string_peek(string,  0))) string = string_skip(string, 1);
	return string;
}

String string_trim_right_spaces(String string)
{
	while (char_is_whitespace(string_peek(string, -1))) string = string_chop(string, 1);
	return string;
}

String string_trim_spaces(String string)
{
	string = string_trim_left_spaces(string);
	string = string_trim_right_spaces(string);
	return string;
}

String string_unquote(String string)
{
	if (string_peek(string,  0) == '"' &&
		string_peek(string, -1) == '"')
	{
		return string_trim(string, 1, 1);
	}
	return string;
}

String string_trim_blank_lines(String string)
{
	isz start = 0;

	for (isz i = 0; i < string.count; i += 1)
	{
		if (char_is_whitespace(string.bytes[i]))
		{
			if (string.bytes[i] == '\n')
			{
				start = i + 1;
			}
		}
		else
		{
			break;
		}
	}

	isz end = string.count;

	for (isz i = string.count - 1; i >= 0; i -= 1)
	{
		if (char_is_whitespace(string.bytes[i]))
		{
			if (string.bytes[i] == '\n')
			{
				end = i + 1;
			}
		}
		else
		{
			break;
		}
	}

	return substring_range(string, start, end);
}

String string_skip(String string, isz amount)
{
	amount = CLAMP(amount, 0, string.count);
	string.bytes += amount;
	string.count -= amount;
	return string;
}

String string_chop(String string, isz amount)
{
	amount = CLAMP(amount, 0, string.count);
	string.count -= amount;
	return string;
}

String string_trim(String string, isz from_start, isz from_end)
{
	string = string_skip(string, from_start);
	string = string_chop(string, from_end);
	return string;
}

String string_head(String string, isz length)
{
	return substring(string, 0, length);
}

String string_tail(String string, isz length)
{
	return substring(string, string.count - length, length);
}

String_Pair string_split_line(String string)
{
	isz j = string_find_first_char(string, '\n', 0);
	isz i = j;
	if (j < string.count)
	{
		if (i > 0 && string.bytes[i - 1] == '\r')
		{
			i -= 1;
		}
	}
	String_Pair result;
	result.l = substring_range(string, 0, i);
	result.r = substring_range(string, j + 1, string.count);
	return result;
}

String_Pair string_split_around(String string, String separator)
{
	isz i = string_find_substring(string, separator, 0);
	String_Pair result;
	result.l = substring_range(string, 0, i);
	result.r = substring_range(string, i + separator.count, string.count);
	return result;
}

String_Pair string_split_around_char(String string, char c)
{
	isz i = string_find_first_char(string, c, 0);
	String_Pair result;
	result.l = substring_range(string, 0, i);
	result.r = substring_range(string, i + 1, string.count);
	return result;
}

String_Pair string_split_word(String string)
{
	isz i, j;
	for (i = 0; i < string.count &&  char_is_whitespace(string.bytes[i]); i += 1);
	for (j = i; j < string.count && !char_is_whitespace(string.bytes[j]); j += 1);

	if (i < j)
	{
		String_Pair result;
		result.l = substring_range(string, i, j);
		result.r = substring_range(string, j, string.count);
		return result;
	}

	String_Pair result = {0};
	result.r = string;
	return result;
}

String_Pair string_split_identifier(String string)
{
	isz i, j;
	for (i = 0; i < string.count &&  char_is_whitespace(string.bytes[i]); i += 1);

	j = i;
	if (j < string.count && (char_is_alphabetic(string.bytes[j]) || string.bytes[j] == '_'))
	{
		j += 1;

		while (j < string.count && (char_is_alphanumeric(string.bytes[j]) || string.bytes[j] == '_'))
		{
			j += 1;
		}
	}

	if (i < j)
	{
		String_Pair result;
		result.l = substring_range(string, i, j);
		result.r = substring_range(string, j, string.count);
		return result;
	}

	String_Pair result = {0};
	result.r = string;
	return result;
}

// iterators (modify iter on each call)
String string_iter_word(String *iter)
{
	String_Pair split = string_split_word(*iter);
	*iter = split.r;
	return split.l;
}

String string_iter_line(String *iter)
{
	String_Pair split = string_split_line(*iter);
	*iter = split.r;
	return split.l;
}

Parse_Number_Result string_parse_u64(String string)
{
	Parse_Number_Result result = {0};

	char *at  = string.bytes;
	char *end = string.bytes + string.count;

	while (at < end && char_is_whitespace(at[0]))
	{
		at += 1;
	}

    i64 sign = 1;
	while (at < end)
	{
		/**/ if (at[0] == '-') { sign *= -1; at += 1; }
		else if (at[0] == '+') {             at += 1; }
		else break;
	}

	if (sign == -1)
	{
		result.overflowed = -1;
		return result;
	}

    i64 base = 10;
	if (at[0] == '0' && at + 1 < end)
    { 
		if (at[1] >= '1' && at[1] <= '9')
		{
			base = 8;
			at += 1;
		}
		else if (at[1] == 'x' || at[1] == 'X')
		{
			base = 16;
			at += 2;
		}
		else if (at[1] == 'b' || at[1] == 'B')
		{
			base = 2;
			at += 2;
		}
		else
		{
			base = -1;
		}
    }

	if (base > 0)
	{
		char *value_start = at;

		i8 overflow = 0;

		u64 value = 0;
		while (at < end)
		{
			i64 digit = digit_from_char_ex(at[0], base);
			if (digit == -1)
			{
				break;
			}
			at += 1;

			// handle overflow
			if (value > (UINT64_MAX - (u64)digit) / (u64)base)
			{
				value    = UINT64_MAX;
				overflow = 1;
				break;
			}

			value *= base;
			value += digit;
		}

		if (value_start != at)
		{
			result.is_valid   = true;
			result.value_u64  = value;
			result.overflowed = overflow;
			result.advance    = at - string.bytes;
		}
	}

	return result;
}

Parse_Number_Result string_parse_i64(String string)
{
	String iter = string;

	iter = string_trim_left_spaces(iter);

	i32 sign = 1;
	while (!string_empty(iter))
	{
		/**/ if (string.bytes[0] == '-') { sign *= -1; iter = string_skip(iter, 1); }
		else if (string.bytes[0] == '+') {             iter = string_skip(iter, 1); }
		else break;
	}

	Parse_Number_Result result = string_parse_u64(string);
	result.advance += iter.bytes - string.bytes;

	u64 max_value = (u64)INT64_MAX;
	if (sign == -1) max_value += 1;

	if (result.value_u64 >= max_value)
	{
		result.value_i64 = sign == 1 ? INT64_MAX : INT64_MIN;

		if (result.value_u64 > max_value)
		{
			result.overflowed = sign;
		}
	}
	else
	{
		result.value_i64 = sign * (i64)result.value_u64;
	}

	return result;
}

Parse_Number_Result string_parse_u32(String string)
{
	Parse_Number_Result result = string_parse_u64(string);
	if (result.value_u64 > UINT32_MAX)
	{
		result.value_u64  = UINT32_MAX;
		result.overflowed = 1;
	}
	return result;
}

Parse_Number_Result string_parse_i32(String string)
{
	Parse_Number_Result result = string_parse_i64(string);
	if (result.value_i64 > INT32_MAX)
	{
		result.value_i64  = INT32_MAX;
		result.overflowed = 1;
	}
	else if (result.value_i64 < INT32_MIN)
	{
		result.value_i64  = INT32_MIN;
		result.overflowed = -1;
	}
	return result;
}

Parse_Number_Result string_parse_u16(String string)
{
	Parse_Number_Result result = string_parse_u64(string);
	if (result.value_u64 > UINT16_MAX)
	{
		result.value_u64  = UINT16_MAX;
		result.overflowed = 1;
	}
	return result;
}

Parse_Number_Result string_parse_i16(String string)
{
	Parse_Number_Result result = string_parse_i64(string);
	if (result.value_i64 > INT16_MAX)
	{
		result.value_i64  = INT16_MAX;
		result.overflowed = 1;
	}
	else if (result.value_i64 < INT16_MIN)
	{
		result.value_i64  = INT16_MIN;
		result.overflowed = -1;
	}
	return result;
}

Parse_Number_Result string_parse_u8(String string)
{
	Parse_Number_Result result = string_parse_u64(string);
	if (result.value_u64 > UINT8_MAX)
	{
		result.value_u64  = UINT8_MAX;
		result.overflowed = 1;
	}
	return result;
}

Parse_Number_Result string_parse_i8(String string)
{
	Parse_Number_Result result = string_parse_i64(string);
	if (result.value_i64 > INT8_MAX)
	{
		result.value_i64  = INT8_MAX;
		result.overflowed = 1;
	}
	else if (result.value_i64 < INT8_MIN)
	{
		result.value_i64  = INT8_MIN;
		result.overflowed = -1;
	}
	return result;
}

Parse_Number_Result string_parse_f64(String string)
{
	// TODO(daniel): Custom implementation

	String iter = string_trim_left_spaces(string);

	char buffer[64];
	string_null_terminate_into_buffer(buffer, sizeof(buffer), iter);

	char *end = NULL;
	f64 value = strtod(buffer, &end);

	Parse_Number_Result result = {0};
	result.is_valid  = end != buffer;
	result.value_f64 = value;

	if (result.is_valid)
	{
		result.advance = (iter.bytes - string.bytes) + (end - buffer);
	}

	return result;
}

Parse_Number_Result string_parse_f32(String string)
{
	Parse_Number_Result result = string_parse_f64(string);
	result.value_f32 = (f32)result.value_f64;
	return result;
}

//
// String Builder
//

fn_local String_Builder_Chunk *sb_guarantee_chunk(String_Builder *sb, isz desired_size)
{
	String_Builder_Chunk *result = sb->last_chunk;

	if (result == NULL || result->count >= result->capacity)
	{
		String_Builder_Chunk *new_chunk = sb->first_free_chunk;

		if (new_chunk == NULL || new_chunk->capacity < desired_size)
		{
			isz chunk_capacity = round_up_pow2(desired_size, DC_STRING_BUILDER_DEFAULT_CHUNK_SIZE);

			new_chunk = arena_alloc_struct_nozero(sb->arena, String_Builder_Chunk);
			new_chunk->next     = NULL;
			new_chunk->capacity = (i32)chunk_capacity;
			new_chunk->bytes    = (char *)arena_alloc_nozero(sb->arena, chunk_capacity, 16);
		}
		else
		{
			sb->first_free_chunk = new_chunk->next;
		}

		new_chunk->count = 0;

		sll_push_back(sb->first_chunk, sb->last_chunk, new_chunk);

		result = new_chunk;
	}

	return result;
}

void sb_init(String_Builder *sb, Arena *arena)
{
	zero_struct(sb);
	sb->arena = arena;
}

isz sb_appendc(String_Builder *sb, char c)
{
	String_Builder_Chunk *chunk = sb_guarantee_chunk(sb, 1);
	chunk->bytes[chunk->count] = c;
	chunk->count    += 1;
	sb->total_count += 1;
	return 1;
}

isz sb_appends(String_Builder *sb, String s)
{
	char *at      = s.bytes;
	isz   to_copy = s.count;
	while (to_copy > 0)
	{
		String_Builder_Chunk *chunk = sb_guarantee_chunk(sb, to_copy);
		isz space  = chunk->capacity - chunk->count;
		isz copied = MIN(to_copy, space);
		copy_bytes(&chunk->bytes[chunk->count], at, copied);
		chunk->count += copied;
		at           += copied;
		to_copy      -= copied;
	}
	sb->total_count += s.count;
	return s.count;
}

fn_local char *sb_sprintf_cb(const char *buf, void *user, int len)
{
	String_Builder *sb = (String_Builder *)user;
	sb_appends(sb, string((char *)buf, len));
	return (char *)buf;
}

isz sb_appendf(String_Builder *sb, char const *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	isz result = sb_appendf_va(sb, fmt, args);

	va_end(args);

	return result;
}

isz sb_appendf_va(String_Builder *sb, char const *fmt, va_list args)
{
	isz start_count = sb->total_count;

	char buffer[512];
	stbsp_vsprintfcb(sb_sprintf_cb, sb, buffer, fmt, args);

	isz end_count = sb->total_count;
	return end_count - start_count;
}

isz sb_appendc_n(String_Builder *sb, char c, isz n)
{
	isz result = 0;
	for (isz i = 0; i < n; i += 1)
	{
		result += sb_appendc(sb, c);
	}
	return result;
}

isz sb_append_spaces(String_Builder *sb, isz n)
{
	return sb_appendc_n(sb, ' ', n);
}

isz sb_append_indentation(String_Builder *sb, isz depth)
{
	isz result;
	if (sb->indent_space_count > 0)
	{
		result = sb_appendc_n(sb, ' ', depth * sb->indent_space_count);
	}
	else
	{
		result = sb_appendc_n(sb, '\t', depth);
	}
	return result;
}

isz sb_append_line_indent(String_Builder *sb)
{
	return sb_append_indentation(sb, sb->line_indent_depth);
}

isz sb_line(String_Builder *sb, char const *fmt, ...)
{
	sb_append_line_indent(sb);

	va_list args;
	va_start(args, fmt);

	isz result = sb_appendf_va(sb, fmt, args);

	va_end(args);

	sb_newline(sb);

	return result;
}

isz sb_newline(String_Builder *sb)
{
	if (sb->use_crlf)
	{
		return sb_appends(sb, S("\r\n"));
	}

	return sb_appendc(sb, '\n');
}

void sb_push_indent(String_Builder *sb, isz depth)
{
	depth = MAX(0, depth);
	sb->line_indent_depth += depth;
}

void sb_pop_indent(String_Builder *sb, isz depth)
{
	depth = MAX(0, depth);
	sb->line_indent_depth = MAX(0, sb->line_indent_depth - depth);
}

isz sb_append_reindented(String_Builder *sb, String str, isz indent_delta, String_Reindent_Flags flags)
{
	if (flags & String_Reindent_trim_blank_lines)
	{
		str = string_trim_blank_lines(str);
	}

	// TODO: Handle mixed indentation robustly
	isz min_indentation = INT64_MAX;
	for (String_EachLine(line, str))
	{
		isz first_non_whitespace = INT64_MAX;
		for (isz i = 0; i < line.count; i += 1)
		{
			if (!char_is_whitespace(line.bytes[i]))
			{
				if (!(flags & String_Reindent_left_justify_preprocessor_defines) || line.bytes[i] != '#')
				{
					first_non_whitespace = i;
				}
				break;
			}
		}

		if (min_indentation > first_non_whitespace)
		{
			min_indentation = first_non_whitespace;
		}
	}

	if (min_indentation == INT64_MAX)
	{
		min_indentation = 0;
	}

	isz result = 0;
	isz depth  = MAX(0, sb->line_indent_depth + indent_delta);

	for (String_EachLine(line, str))
	{
		isz first_non_space = string_find_first_non_whitespace(line);

		String stripped;
		if ((flags & String_Reindent_left_justify_preprocessor_defines) && string_peek(line, first_non_space) == '#')
		{
			stripped = string_skip(line, first_non_space);
		}
		else
		{
			stripped = string_skip(line, min_indentation);

			for (isz i = 0; i < depth; i += 1)
			{
				result += sb_appendc(sb, '\t');
			}
		}

		result += sb_appends(sb, stripped);
		result += sb_appendc(sb, '\n');
	}

	return result;
}

void sb_reset(String_Builder *sb)
{
	sb->total_count       = 0;
	sb->line_indent_depth = 0;

	sb->last_chunk->next = sb->first_free_chunk;
	sb->first_free_chunk = sb->first_chunk->next;

	sb->first_chunk = NULL;
	sb->last_chunk  = NULL;
}

String sb_flatten(Arena *arena, String_Builder *sb)
{
	String result = string_allocate(arena, sb->total_count);

	isz at = 0;
	for (String_Builder_Chunk *chunk = sb->first_chunk; chunk; chunk = chunk->next)
	{
		copy_bytes(&result.bytes[at], &chunk->bytes[0], chunk->count);
		at += chunk->count;
	}

	return result;
}

isz sb_append_bytes(String_Builder *sb, void *bytes, isz n)
{
	return sb_appends(sb, string((char *)bytes, n));
}

isz sb_append_bin_i8(String_Builder *sb, i8 v) { return sb_append_bytes(sb, &v, sizeof(v)); }
isz sb_append_bin_i16(String_Builder *sb, i16 v) { return sb_append_bytes(sb, &v, sizeof(v)); }
isz sb_append_bin_i32(String_Builder *sb, i32 v) { return sb_append_bytes(sb, &v, sizeof(v)); }
isz sb_append_bin_i64(String_Builder *sb, i64 v) { return sb_append_bytes(sb, &v, sizeof(v)); }
isz sb_append_bin_u8(String_Builder *sb, u8 v) { return sb_append_bytes(sb, &v, sizeof(v)); }
isz sb_append_bin_u16(String_Builder *sb, u16 v) { return sb_append_bytes(sb, &v, sizeof(v)); }
isz sb_append_bin_u32(String_Builder *sb, u32 v) { return sb_append_bytes(sb, &v, sizeof(v)); }
isz sb_append_bin_u64(String_Builder *sb, u64 v) { return sb_append_bytes(sb, &v, sizeof(v)); }
isz sb_append_bin_f32(String_Builder *sb, f32 v) { return sb_append_bytes(sb, &v, sizeof(v)); }
isz sb_append_bin_f64(String_Builder *sb, f64 v) { return sb_append_bytes(sb, &v, sizeof(v)); }

bool sb_verify(String_Builder *sb)
{
	isz total_count = 0;
	for (String_Builder_Chunk *chunk = sb->first_chunk; chunk; chunk = chunk->next)
	{
		total_count += chunk->count;
	}
	return sb->total_count == total_count;
}

//
// File System
//

#include <stdio.h>

#if defined(_WIN32)

/* ported from cbloom's robust win32 io */

enum
{
	DC_WIN32_IO_NOTALIGNED           = 1,
	DC_WIN32_IO_UNBUFFERED_ALIGNMENT = 4096,
	DC_WIN32_IO_MAX_RETIRES          = 10,
	DC_WIN32_IO_MAX_SINGLE_IO_SIZE   = 1 << 24,
};

typedef enum Win32_Io_Status
{
	Win32_Io_Status_started_async,
	Win32_Io_Status_done_sync,
	Win32_Io_Status_done_sync_eof,
	Win32_Io_Status_error,
} Win32_Io_Status_Status;

fn_local Win32_Io_Status_Status win32_async_read(
	HANDLE      handle,
	uint64_t    offset,
	uint32_t    size,
	void       *buffer,
	OVERLAPPED *overlapped,
	uint32_t   *read_size)
{
	zero_struct(overlapped);

	LARGE_INTEGER offset_;
	offset_.QuadPart = offset;
	overlapped->Offset     = offset_.LowPart;
	overlapped->OffsetHigh = offset_.HighPart;

	// There is a bug in Windows XP where ReadFile/WriteFile returns an error but doesn't set LastError,
	// this guards against that, in case this code ever runs on XP, I guess...
	SetLastError(0);

	DWORD bytes_read = 0;
	BOOL  read_ok    = ReadFile(handle, buffer, size, &bytes_read, overlapped);

	*read_size = bytes_read;

	Win32_Io_Status_Status result = Win32_Io_Status_error;

	if (read_ok)
	{
		// weird case, ReadFile returned done immediately - not overlapped? not async!
		zero_struct(overlapped);

		DWORD error = GetLastError();

		if (error == ERROR_HANDLE_EOF)
		{
			result = Win32_Io_Status_done_sync_eof;
		}
		else
		{
			result = Win32_Io_Status_done_sync;
		}
	}
	else
	{
		DWORD error = GetLastError();

		if (error == ERROR_IO_PENDING)
		{
			// started async read successfully
			result = Win32_Io_Status_started_async;
		}
		else if (error == ERROR_HANDLE_EOF)
		{
			// we were at EOF at start of read - not async!
			zero_struct(overlapped);

			result = Win32_Io_Status_done_sync_eof;
		}
		else
		{
			// TODO(daniel): Error reporting!
		}
	}

	return result;
}

fn_local Win32_Io_Status_Status win32_async_write(
	HANDLE      handle,
	uint64_t    offset,
	uint32_t    size,
	const void *buffer,
	OVERLAPPED *overlapped,
	uint32_t   *written_size)
{
	zero_struct(overlapped);

	LARGE_INTEGER offset_;
	offset_.QuadPart = offset;

	overlapped->Offset     = offset_.LowPart;
	overlapped->OffsetHigh = offset_.HighPart;

	// There is a bug in Windows XP where ReadFile/WriteFile returns an error but doesn't set LastError,
	// this guards against that, in case this code ever runs on XP, I guess...
	SetLastError(0);

	DWORD bytes_written = 0;
	BOOL  write_ok      = WriteFile(handle, buffer, size, &bytes_written, overlapped);

	*written_size = bytes_written;

	Win32_Io_Status_Status result = Win32_Io_Status_error;

	if (write_ok)
	{
		// WriteFile returned done immediately - not overlapped?

		// Almost all writes go through this path:
		// http://support.microsoft.com/kb/156932

		// for writes to be actually async you must have:
		//   used unbuffered IO
		//   pre-extended the size of the file
		//   validated that range with SetFileValidData
		// and even that is no guarantee

		// not async!
		zero_struct(overlapped);
		
		result = Win32_Io_Status_done_sync;
	}
	else
	{
		DWORD error = GetLastError();

		if (error == ERROR_IO_PENDING)
		{
			// good successful async write
			result = Win32_Io_Status_started_async;
		}
		else
		{
			// TODO(daniel): Error reporting!
		}
	}

	return result;
}

fn_local Win32_Io_Status_Status win32_get_async_result(HANDLE handle, OVERLAPPED *overlapped, uint32_t *out_size)
{
	Win32_Io_Status_Status result = Win32_Io_Status_error;

	DWORD size = 0;

	// first check the result with no wait - this also resets the event so that the next call to GOR works
	if (GetOverlappedResult(handle, overlapped, &size, FALSE))
	{
		if (size > 0)
		{
			result = Win32_Io_Status_done_sync;
		}
	}
	else
	{
		// calling GOR with TRUE will yield of thread if the IO is still pending
		if (GetOverlappedResult(handle, overlapped, &size, TRUE))
		{
			result = Win32_Io_Status_done_sync;
		}
		else
		{
			DWORD error = GetLastError();

			if (error == ERROR_HANDLE_EOF)
			{
				result = Win32_Io_Status_done_sync_eof;
			}
		}
	}

	*out_size = size;

	return result;
}

fn_local Win32_Io_Status_Status win32_sync_read_sub(
	HANDLE    handle,
	uint64_t  offset,
	uint32_t  size,
	void     *buffer,
	uint32_t *read_size)
{
	if (read_size)
	{
		*read_size = 0;
	}

	Win32_Io_Status_Status result = Win32_Io_Status_error;

	if (size == 0)
	{
		result = Win32_Io_Status_done_sync;
	}
	else
	{
		for (size_t retry_index = 0; retry_index < DC_WIN32_IO_MAX_RETIRES; retry_index++)
		{
			OVERLAPPED async = { 0 };

			uint32_t               bytes_read = 0;
			Win32_Io_Status_Status status     = win32_async_read(handle, offset, size, buffer, &async, &bytes_read);

			if (status == Win32_Io_Status_started_async)
			{
				status = win32_get_async_result(handle, &async, &bytes_read);
			}

			if (status == Win32_Io_Status_done_sync_eof)
			{
				if (read_size)
				{
					*read_size = bytes_read;
				}

				result = status;
				break;
			}
			else if (status == Win32_Io_Status_done_sync)
			{
				if (bytes_read > 0)
				{
					if (read_size)
					{
						*read_size = bytes_read;
					}

					result = status;
					break;
				}

				// else, retry
			}
			else
			{
				dc_assert(status == Win32_Io_Status_error);

				DWORD error = GetLastError();

				if (error == ERROR_NO_SYSTEM_RESOURCES ||
					error == ERROR_NOT_ENOUGH_MEMORY)
				{
					DWORD milliseconds = MIN(1 + (DWORD)retry_index*10, 50);
					Sleep(milliseconds);
				}
				else
				{
					result = Win32_Io_Status_error;
					break;
				}
			}
		}
	}

	return result;
}

fn_local Win32_Io_Status_Status win32_sync_write_sub(
	HANDLE      handle,
	uint64_t    offset,
	uint32_t    size,
	const void *buffer,
	uint32_t   *written_size)
{
	if (written_size)
	{
		*written_size = 0;
	}

	Win32_Io_Status_Status result = Win32_Io_Status_error;

	if (size == 0)
	{
		result = Win32_Io_Status_done_sync;
	}
	else
	{
		for (size_t retry_index = 0; retry_index < DC_WIN32_IO_MAX_RETIRES; retry_index++)
		{
			OVERLAPPED async = { 0 };

			uint32_t               bytes_written = 0;
			Win32_Io_Status_Status status        = win32_async_write(handle, offset, size, buffer, &async, &bytes_written);

			if (status == Win32_Io_Status_started_async)
			{
				status = win32_get_async_result(handle, &async, &bytes_written);
			}

			if (status == Win32_Io_Status_done_sync || status == Win32_Io_Status_done_sync_eof)
			{
				if (written_size)
				{
					*written_size = bytes_written;
				}

				result = status;
				break;
			}
			else
			{
				DWORD error = GetLastError();

				if (error == ERROR_NO_SYSTEM_RESOURCES ||
					error == ERROR_NOT_ENOUGH_MEMORY)
				{
					DWORD milliseconds = MIN(1 + (uint32_t)retry_index*10, 50);
					Sleep(milliseconds);
				}
				else
				{
					result = Win32_Io_Status_error;
					break;
				}
			}
		}
	}

	return result;
}

fn_local bool win32_sync_read(
	HANDLE    handle,
	uint64_t  offset,
	uint64_t  size,
	void     *buffer,
	uint64_t *bytes_read,
	uint32_t  alignment)
{
	if (alignment == 0)
	{
		alignment = 1;
	}

	if ((offset & (alignment - 1)) != 0)
	{
		return false;
	}

	bool result = true;

	char *at = (char *)buffer;

	uint64_t total_bytes_read = 0;

	while (size > 0)
	{
		uint32_t chunk_read_size = (uint32_t)MIN(size, DC_WIN32_IO_MAX_SINGLE_IO_SIZE);

		uint32_t               chunk_bytes_read = 0;
		Win32_Io_Status_Status status           = win32_sync_read_sub(handle, offset, chunk_read_size, at, &chunk_bytes_read);

		if (status == Win32_Io_Status_error)
		{
			result = false;
			break;
		}

		total_bytes_read += chunk_bytes_read;

		if (status == Win32_Io_Status_done_sync_eof)
		{
			result = true;
			break;
		}

		if (chunk_bytes_read == 0)
		{
			// successful read, yet we got no bytes. error!
			result = false;
			break;
		}

		offset += chunk_bytes_read;
		at     += chunk_bytes_read;
		size   -= chunk_bytes_read;

		if ((offset & (alignment - 1)) != 0)
		{
			// If we got off the alignment due to a partial IO, it must be due to EOF, so I guess we succeeded.
			result = true;
			break;
		}
	}

	if (bytes_read)
	{
		*bytes_read = total_bytes_read;
	}

	return result;
}

fn_local bool win32_sync_write(
	HANDLE      handle, 
	uint64_t    offset, 
	uint64_t    size, 
	const void *buffer, 
	uint64_t   *bytes_written, 
	uint32_t    alignment)
{
	if (alignment == 0)
	{
		alignment = 1;
	}

	if ((offset & (alignment - 1)) != 0)
	{
		return false;
	}

	bool result = true;

	const char *at = (char *)buffer;

	uint64_t total_bytes_written = 0;

	while (size > 0)
	{
		uint32_t chunk_write_size = (uint32_t)MIN(size, DC_WIN32_IO_MAX_SINGLE_IO_SIZE);

		uint32_t               chunk_bytes_written = 0;
		Win32_Io_Status_Status status              = win32_sync_write_sub(handle, offset, chunk_write_size, at, &chunk_bytes_written);

		if (status == Win32_Io_Status_error)
		{
			result = false;
			break;
		}

		total_bytes_written += chunk_bytes_written;

		offset += chunk_bytes_written;
		at     += chunk_bytes_written;
		size   -= chunk_bytes_written;

		if (status == Win32_Io_Status_done_sync_eof)
		{
			// EOF during write? might be possible with read-write file in non-append mode?

			if (size == 0)
			{
				result = true; // wrote all bytes
			}
			else
			{
				result = false; // did not write all bytes :(
			}

			break;
		}

		dc_assert(status == Win32_Io_Status_done_sync);

		if (chunk_bytes_written == 0)
		{
			// succeeded, and yet no bytes written? suspicious
			result = false;
			break;
		}

		if ((offset & (alignment - 1)) != 0)
		{
			// If we got off the alignment due to a partial IO, it must be due to EOF, so I guess we succeeded.
			result = true;
			break;
		}
	}

	if (bytes_written)
	{
		*bytes_written = total_bytes_written;
	}

	return result;
}

bool os_write_entire_file(String path, Memory memory)
{
	bool result = true;

	Arena_ScopedTemp {
		String16 path16 = utf16_from_utf8(temp, path);

		HANDLE handle = CreateFileW(path16.bytes, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (handle != INVALID_HANDLE_VALUE)
		{
			uint64_t written;
			result = win32_sync_write(handle, 0, memory.size, memory.bytes, &written, 1);

			CloseHandle(handle);
		}
		else
		{
			result = false;
		}
	}

	return result;
}

Memory os_read_entire_file(Arena *arena, String path)
{
	Memory result = {0};

	Arena_ScopedTemp {
		String16 path16 = utf16_from_utf8(temp, path);

		HANDLE handle = CreateFileW(path16.bytes, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (handle != INVALID_HANDLE_VALUE)
		{
			DWORD file_size_high;
			DWORD file_size_low = GetFileSize(handle, &file_size_high);

			LARGE_INTEGER file_size_;
			file_size_.LowPart  = file_size_low;
			file_size_.HighPart = file_size_high;

			isz file_size = (isz)file_size_.QuadPart;

			if (file_size > 0)
			{
				char *buffer = (char *)arena_alloc_nozero(arena, file_size + 1, 16);

				uint64_t bytes_read;
				bool success = win32_sync_read(handle, 0, file_size, buffer, &bytes_read, 1);

				if (success)
				{
					buffer[bytes_read] = 0;
					result.size = (isz)bytes_read;
					result.bytes = buffer;
				}
			}
		}

		CloseHandle(handle);
	}

	return result;
}

#else // defined(_WIN32)
#error TODO: Other platforms
#endif

/* -----------------------------------------------------------------------------------------------------
   What follows is the implementation of stbsprintf, check the bottom of the file for its license.
   -----------------------------------------------------------------------------------------------------*/

#define stbsp__uint32 unsigned int
#define stbsp__int32 signed int

#ifdef _MSC_VER
#define stbsp__uint64 unsigned __int64
#define stbsp__int64 signed __int64
#else
#define stbsp__uint64 unsigned long long
#define stbsp__int64 signed long long
#endif
#define stbsp__uint16 unsigned short

#ifndef stbsp__uintptr
#if defined(__ppc64__) || defined(__powerpc64__) || defined(__aarch64__) || defined(_M_X64) || defined(__x86_64__) || defined(__x86_64) || defined(__s390x__)
#define stbsp__uintptr stbsp__uint64
#else
#define stbsp__uintptr stbsp__uint32
#endif
#endif

#ifndef STB_SPRINTF_MSVC_MODE // used for MSVC2013 and earlier (MSVC2015 matches GCC)
#if defined(_MSC_VER) && (_MSC_VER < 1900)
#define STB_SPRINTF_MSVC_MODE
#endif
#endif

#ifdef STB_SPRINTF_NOUNALIGNED // define this before inclusion to force stbsp_sprintf to always use aligned accesses
#define STBSP__UNALIGNED(code)
#else
#define STBSP__UNALIGNED(code) code
#endif

#ifndef STB_SPRINTF_NOFLOAT
// internal float utility functions
static stbsp__int32 stbsp__real_to_str(char const **start, stbsp__uint32 *len, char *out, stbsp__int32 *decimal_pos, double value, stbsp__uint32 frac_digits);
static stbsp__int32 stbsp__real_to_parts(stbsp__int64 *bits, stbsp__int32 *expo, double value);
#define STBSP__SPECIAL 0x7000
#endif

static char stbsp__period = '.';
static char stbsp__comma = ',';
static struct
{
   short temp; // force next field to be 2-byte aligned
   char pair[201];
} stbsp__digitpair =
{
  0,
   "00010203040506070809101112131415161718192021222324"
   "25262728293031323334353637383940414243444546474849"
   "50515253545556575859606162636465666768697071727374"
   "75767778798081828384858687888990919293949596979899"
};

STBSP__PUBLICDEF void STB_SPRINTF_DECORATE(set_separators)(char pcomma, char pperiod)
{
   stbsp__period = pperiod;
   stbsp__comma = pcomma;
}

#define STBSP__LEFTJUST 1
#define STBSP__LEADINGPLUS 2
#define STBSP__LEADINGSPACE 4
#define STBSP__LEADING_0X 8
#define STBSP__LEADINGZERO 16
#define STBSP__INTMAX 32
#define STBSP__TRIPLET_COMMA 64
#define STBSP__NEGATIVE 128
#define STBSP__METRIC_SUFFIX 256
#define STBSP__HALFWIDTH 512
#define STBSP__METRIC_NOSPACE 1024
#define STBSP__METRIC_1024 2048
#define STBSP__METRIC_JEDEC 4096
// DF_EDIT_BEGIN: slice-style-strings
#define STBSP__DC_COUNTED_STRING 8192
// DF_EDIT_END

static void stbsp__lead_sign(stbsp__uint32 fl, char *sign)
{
   sign[0] = 0;
   if (fl & STBSP__NEGATIVE) {
      sign[0] = 1;
      sign[1] = '-';
   } else if (fl & STBSP__LEADINGSPACE) {
      sign[0] = 1;
      sign[1] = ' ';
   } else if (fl & STBSP__LEADINGPLUS) {
      sign[0] = 1;
      sign[1] = '+';
   }
}

static STBSP__ASAN stbsp__uint32 stbsp__strlen_limited(char const *s, stbsp__uint32 limit)
{
   char const * sn = s;

   // get up to 4-byte alignment
   for (;;) {
      if (((stbsp__uintptr)sn & 3) == 0)
         break;

      if (!limit || *sn == 0)
         return (stbsp__uint32)(sn - s);

      ++sn;
      --limit;
   }

   // scan over 4 bytes at a time to find terminating 0
   // this will intentionally scan up to 3 bytes past the end of buffers,
   // but becase it works 4B aligned, it will never cross page boundaries
   // (hence the STBSP__ASAN markup; the over-read here is intentional
   // and harmless)
   while (limit >= 4) {
      stbsp__uint32 v = *(stbsp__uint32 *)sn;
      // bit hack to find if there's a 0 byte in there
      if ((v - 0x01010101) & (~v) & 0x80808080UL)
         break;

      sn += 4;
      limit -= 4;
   }

   // handle the last few characters to find actual size
   while (limit && *sn) {
      ++sn;
      --limit;
   }

   return (stbsp__uint32)(sn - s);
}

STBSP__PUBLICDEF int STB_SPRINTF_DECORATE(vsprintfcb)(STBSP_SPRINTFCB *callback, void *user, char *buf, char const *fmt, va_list va)
{
   static char hex[] = "0123456789abcdefxp";
   static char hexu[] = "0123456789ABCDEFXP";
   char *bf;
   char const *f;
   int tlen = 0;

   bf = buf;
   f = fmt;
   for (;;) {
      stbsp__int32 fw, pr, tz;
      stbsp__uint32 fl;

      // macros for the callback buffer stuff
      #define stbsp__chk_cb_bufL(bytes)                        \
         {                                                     \
            int len = (int)(bf - buf);                         \
            if ((len + (bytes)) >= STB_SPRINTF_MIN) {          \
               tlen += len;                                    \
               if (0 == (bf = buf = callback(buf, user, len))) \
                  goto done;                                   \
            }                                                  \
         }
      #define stbsp__chk_cb_buf(bytes)    \
         {                                \
            if (callback) {               \
               stbsp__chk_cb_bufL(bytes); \
            }                             \
         }
      #define stbsp__flush_cb()                      \
         {                                           \
            stbsp__chk_cb_bufL(STB_SPRINTF_MIN - 1); \
         } // flush if there is even one byte in the buffer
      #define stbsp__cb_buf_clamp(cl, v)                \
         cl = v;                                        \
         if (callback) {                                \
            int lg = STB_SPRINTF_MIN - (int)(bf - buf); \
            if (cl > lg)                                \
               cl = lg;                                 \
         }

      // fast copy everything up to the next % (or end of string)
      for (;;) {
         while (((stbsp__uintptr)f) & 3) {
         schk1:
            if (f[0] == '%')
               goto scandd;
         schk2:
            if (f[0] == 0)
               goto endfmt;
            stbsp__chk_cb_buf(1);
            *bf++ = f[0];
            ++f;
         }
         for (;;) {
            // Check if the next 4 bytes contain %(0x25) or end of string.
            // Using the 'hasless' trick:
            // https://graphics.stanford.edu/~seander/bithacks.html#HasLessInWord
            stbsp__uint32 v, c;
            v = *(stbsp__uint32 *)f;
            c = (~v) & 0x80808080;
            if (((v ^ 0x25252525) - 0x01010101) & c)
               goto schk1;
            if ((v - 0x01010101) & c)
               goto schk2;
            if (callback)
               if ((STB_SPRINTF_MIN - (int)(bf - buf)) < 4)
                  goto schk1;
            #ifdef STB_SPRINTF_NOUNALIGNED
                if(((stbsp__uintptr)bf) & 3) {
                    bf[0] = f[0];
                    bf[1] = f[1];
                    bf[2] = f[2];
                    bf[3] = f[3];
                } else
            #endif
            {
                *(stbsp__uint32 *)bf = v;
            }
            bf += 4;
            f += 4;
         }
      }
   scandd:

      ++f;

      // ok, we have a percent, read the modifiers first
      fw = 0;
      pr = -1;
      fl = 0;
      tz = 0;

      // flags
      for (;;) {
         switch (f[0]) {
         // if we have left justify
         case '-':
            fl |= STBSP__LEFTJUST;
            ++f;
            continue;
         // if we have leading plus
         case '+':
            fl |= STBSP__LEADINGPLUS;
            ++f;
            continue;
         // if we have leading space
         case ' ':
            fl |= STBSP__LEADINGSPACE;
            ++f;
            continue;
         // if we have leading 0x
         case '#':
            fl |= STBSP__LEADING_0X;
            ++f;
            continue;
         // if we have thousand commas
         case '\'':
            fl |= STBSP__TRIPLET_COMMA;
            ++f;
            continue;
         // if we have kilo marker (none->kilo->kibi->jedec)
         case '$':
            if (fl & STBSP__METRIC_SUFFIX) {
               if (fl & STBSP__METRIC_1024) {
                  fl |= STBSP__METRIC_JEDEC;
               } else {
                  fl |= STBSP__METRIC_1024;
               }
            } else {
               fl |= STBSP__METRIC_SUFFIX;
            }
            ++f;
            continue;
         // if we don't want space between metric suffix and number
         case '_':
            fl |= STBSP__METRIC_NOSPACE;
            ++f;
            continue;
         // if we have leading zero
         case '0':
            fl |= STBSP__LEADINGZERO;
            ++f;
            goto flags_done;
         default: goto flags_done;
         }
      }
   flags_done:

      // get the field width
      if (f[0] == '*') {
         fw = va_arg(va, stbsp__uint32);
         ++f;
      } else {
         while ((f[0] >= '0') && (f[0] <= '9')) {
            fw = fw * 10 + f[0] - '0';
            f++;
         }
      }
      // get the precision
      if (f[0] == '.') {
         ++f;
         if (f[0] == '*') {
            pr = va_arg(va, stbsp__uint32);
            ++f;
         } else {
            pr = 0;
            while ((f[0] >= '0') && (f[0] <= '9')) {
               pr = pr * 10 + f[0] - '0';
               f++;
            }
         }
      }

      // handle integer size overrides
      switch (f[0]) {
      // are we halfwidth?
      case 'h':
         fl |= STBSP__HALFWIDTH;
         ++f;
         if (f[0] == 'h')
            ++f;  // QUARTERWIDTH
         break;
      // are we 64-bit (unix style)
      case 'l':
         fl |= ((sizeof(long) == 8) ? STBSP__INTMAX : 0);
         ++f;
         if (f[0] == 'l') {
            fl |= STBSP__INTMAX;
            ++f;
         }
         break;
// DC_EDIT_BEGIN: slice-style-strings
	  case 'c': // c for "counted" string
	     fl |= STBSP__DC_COUNTED_STRING;
		 ++f;
		 break;
// DC_EDIT_END
      // are we 64-bit on intmax? (c99)
      case 'j':
         fl |= (sizeof(size_t) == 8) ? STBSP__INTMAX : 0;
         ++f;
         break;
      // are we 64-bit on size_t or ptrdiff_t? (c99)
      case 'z':
         fl |= (sizeof(ptrdiff_t) == 8) ? STBSP__INTMAX : 0;
         ++f;
         break;
      case 't':
         fl |= (sizeof(ptrdiff_t) == 8) ? STBSP__INTMAX : 0;
         ++f;
         break;
      // are we 64-bit (msft style)
      case 'I':
         if ((f[1] == '6') && (f[2] == '4')) {
            fl |= STBSP__INTMAX;
            f += 3;
         } else if ((f[1] == '3') && (f[2] == '2')) {
            f += 3;
         } else {
            fl |= ((sizeof(void *) == 8) ? STBSP__INTMAX : 0);
            ++f;
         }
         break;
      default: break;
      }

      // handle each replacement
      switch (f[0]) {
         #define STBSP__NUMSZ 512 // big enough for e308 (with commas) or e-307
         char num[STBSP__NUMSZ];
         char lead[8];
         char tail[8];
         char *s;
         char const *h;
         stbsp__uint32 l, n, cs;
         stbsp__uint64 n64;
#ifndef STB_SPRINTF_NOFLOAT
         double fv;
#endif
         stbsp__int32 dp;
         char const *sn;

      case 's':
         // get the string
		 // DF_EDIT_BEGIN: slice-style-strings
		 if (fl & STBSP__DC_COUNTED_STRING)
		 {
		    struct stbsp__slice { char *data; stbsp__uint64 count; };
		    struct stbsp__slice slice = va_arg(va, struct stbsp__slice);
		    s = slice.data;
		    if (s == 0)
		        s = (char *)"null";
		    l = (pr >= 0 ? pr : (stbsp__uint32)(slice.count < ~0u ? slice.count : ~0u));
		 }
		 else
		 {
            s = va_arg(va, char *);
            if (s == 0)
               s = (char *)"null";
            // get the length, limited to desired precision
            // always limit to ~0u chars since our counts are 32b
            l = stbsp__strlen_limited(s, (pr >= 0) ? pr : ~0u);
		 }
		 // DF_EDIT_END
         lead[0] = 0;
         tail[0] = 0;
         pr = 0;
         dp = 0;
         cs = 0;
         // copy the string in
         goto scopy;

      case 'c': // char
         // get the character
         s = num + STBSP__NUMSZ - 1;
         *s = (char)va_arg(va, int);
         l = 1;
         lead[0] = 0;
         tail[0] = 0;
         pr = 0;
         dp = 0;
         cs = 0;
         goto scopy;

      case 'n': // weird write-bytes specifier
      {
         int *d = va_arg(va, int *);
         *d = tlen + (int)(bf - buf);
      } break;

#ifdef STB_SPRINTF_NOFLOAT
      case 'A':              // float
      case 'a':              // hex float
      case 'G':              // float
      case 'g':              // float
      case 'E':              // float
      case 'e':              // float
      case 'f':              // float
         va_arg(va, double); // eat it
         s = (char *)"No float";
         l = 8;
         lead[0] = 0;
         tail[0] = 0;
         pr = 0;
         cs = 0;
         STBSP__NOTUSED(dp);
         goto scopy;
#else
      case 'A': // hex float
      case 'a': // hex float
         h = (f[0] == 'A') ? hexu : hex;
         fv = va_arg(va, double);
         if (pr == -1)
            pr = 6; // default is 6
         // read the double into a string
         if (stbsp__real_to_parts((stbsp__int64 *)&n64, &dp, fv))
            fl |= STBSP__NEGATIVE;

         s = num + 64;

         stbsp__lead_sign(fl, lead);

         if (dp == -1023)
            dp = (n64) ? -1022 : 0;
         else
            n64 |= (((stbsp__uint64)1) << 52);
         n64 <<= (64 - 56);
         if (pr < 15)
            n64 += ((((stbsp__uint64)8) << 56) >> (pr * 4));
// add leading chars

#ifdef STB_SPRINTF_MSVC_MODE
         *s++ = '0';
         *s++ = 'x';
#else
         lead[1 + lead[0]] = '0';
         lead[2 + lead[0]] = 'x';
         lead[0] += 2;
#endif
         *s++ = h[(n64 >> 60) & 15];
         n64 <<= 4;
         if (pr)
            *s++ = stbsp__period;
         sn = s;

         // print the bits
         n = pr;
         if (n > 13)
            n = 13;
         if (pr > (stbsp__int32)n)
            tz = pr - n;
         pr = 0;
         while (n--) {
            *s++ = h[(n64 >> 60) & 15];
            n64 <<= 4;
         }

         // print the expo
         tail[1] = h[17];
         if (dp < 0) {
            tail[2] = '-';
            dp = -dp;
         } else
            tail[2] = '+';
         n = (dp >= 1000) ? 6 : ((dp >= 100) ? 5 : ((dp >= 10) ? 4 : 3));
         tail[0] = (char)n;
         for (;;) {
            tail[n] = '0' + dp % 10;
            if (n <= 3)
               break;
            --n;
            dp /= 10;
         }

         dp = (int)(s - sn);
         l = (int)(s - (num + 64));
         s = num + 64;
         cs = 1 + (3 << 24);
         goto scopy;

      case 'G': // float
      case 'g': // float
         h = (f[0] == 'G') ? hexu : hex;
         fv = va_arg(va, double);
         if (pr == -1)
            pr = 6;
         else if (pr == 0)
            pr = 1; // default is 6
         // read the double into a string
         if (stbsp__real_to_str(&sn, &l, num, &dp, fv, (pr - 1) | 0x80000000))
            fl |= STBSP__NEGATIVE;

         // clamp the precision and delete extra zeros after clamp
         n = pr;
         if (l > (stbsp__uint32)pr)
            l = pr;
         while ((l > 1) && (pr) && (sn[l - 1] == '0')) {
            --pr;
            --l;
         }

         // should we use %e
         if ((dp <= -4) || (dp > (stbsp__int32)n)) {
            if (pr > (stbsp__int32)l)
               pr = l - 1;
            else if (pr)
               --pr; // when using %e, there is one digit before the decimal
            goto doexpfromg;
         }
         // this is the insane action to get the pr to match %g semantics for %f
         if (dp > 0) {
            pr = (dp < (stbsp__int32)l) ? l - dp : 0;
         } else {
            pr = -dp + ((pr > (stbsp__int32)l) ? (stbsp__int32) l : pr);
         }
         goto dofloatfromg;

      case 'E': // float
      case 'e': // float
         h = (f[0] == 'E') ? hexu : hex;
         fv = va_arg(va, double);
         if (pr == -1)
            pr = 6; // default is 6
         // read the double into a string
         if (stbsp__real_to_str(&sn, &l, num, &dp, fv, pr | 0x80000000))
            fl |= STBSP__NEGATIVE;
      doexpfromg:
         tail[0] = 0;
         stbsp__lead_sign(fl, lead);
         if (dp == STBSP__SPECIAL) {
            s = (char *)sn;
            cs = 0;
            pr = 0;
            goto scopy;
         }
         s = num + 64;
         // handle leading chars
         *s++ = sn[0];

         if (pr)
            *s++ = stbsp__period;

         // handle after decimal
         if ((l - 1) > (stbsp__uint32)pr)
            l = pr + 1;
         for (n = 1; n < l; n++)
            *s++ = sn[n];
         // trailing zeros
         tz = pr - (l - 1);
         pr = 0;
         // dump expo
         tail[1] = h[0xe];
         dp -= 1;
         if (dp < 0) {
            tail[2] = '-';
            dp = -dp;
         } else
            tail[2] = '+';
#ifdef STB_SPRINTF_MSVC_MODE
         n = 5;
#else
         n = (dp >= 100) ? 5 : 4;
#endif
         tail[0] = (char)n;
         for (;;) {
            tail[n] = '0' + dp % 10;
            if (n <= 3)
               break;
            --n;
            dp /= 10;
         }
         cs = 1 + (3 << 24); // how many tens
         goto flt_lead;

      case 'f': // float
         fv = va_arg(va, double);
      doafloat:
         // do kilos
         if (fl & STBSP__METRIC_SUFFIX) {
            double divisor;
            divisor = 1000.0f;
            if (fl & STBSP__METRIC_1024)
               divisor = 1024.0;
            while (fl < 0x4000000) {
               if ((fv < divisor) && (fv > -divisor))
                  break;
               fv /= divisor;
               fl += 0x1000000;
            }
         }
         if (pr == -1)
            pr = 6; // default is 6
         // read the double into a string
         if (stbsp__real_to_str(&sn, &l, num, &dp, fv, pr))
            fl |= STBSP__NEGATIVE;
      dofloatfromg:
         tail[0] = 0;
         stbsp__lead_sign(fl, lead);
         if (dp == STBSP__SPECIAL) {
            s = (char *)sn;
            cs = 0;
            pr = 0;
            goto scopy;
         }
         s = num + 64;

         // handle the three decimal varieties
         if (dp <= 0) {
            stbsp__int32 i;
            // handle 0.000*000xxxx
            *s++ = '0';
            if (pr)
               *s++ = stbsp__period;
            n = -dp;
            if ((stbsp__int32)n > pr)
               n = pr;
            i = n;
            while (i) {
               if ((((stbsp__uintptr)s) & 3) == 0)
                  break;
               *s++ = '0';
               --i;
            }
            while (i >= 4) {
               *(stbsp__uint32 *)s = 0x30303030;
               s += 4;
               i -= 4;
            }
            while (i) {
               *s++ = '0';
               --i;
            }
            if ((stbsp__int32)(l + n) > pr)
               l = pr - n;
            i = l;
            while (i) {
               *s++ = *sn++;
               --i;
            }
            tz = pr - (n + l);
            cs = 1 + (3 << 24); // how many tens did we write (for commas below)
         } else {
            cs = (fl & STBSP__TRIPLET_COMMA) ? ((600 - (stbsp__uint32)dp) % 3) : 0;
            if ((stbsp__uint32)dp >= l) {
               // handle xxxx000*000.0
               n = 0;
               for (;;) {
                  if ((fl & STBSP__TRIPLET_COMMA) && (++cs == 4)) {
                     cs = 0;
                     *s++ = stbsp__comma;
                  } else {
                     *s++ = sn[n];
                     ++n;
                     if (n >= l)
                        break;
                  }
               }
               if (n < (stbsp__uint32)dp) {
                  n = dp - n;
                  if ((fl & STBSP__TRIPLET_COMMA) == 0) {
                     while (n) {
                        if ((((stbsp__uintptr)s) & 3) == 0)
                           break;
                        *s++ = '0';
                        --n;
                     }
                     while (n >= 4) {
                        *(stbsp__uint32 *)s = 0x30303030;
                        s += 4;
                        n -= 4;
                     }
                  }
                  while (n) {
                     if ((fl & STBSP__TRIPLET_COMMA) && (++cs == 4)) {
                        cs = 0;
                        *s++ = stbsp__comma;
                     } else {
                        *s++ = '0';
                        --n;
                     }
                  }
               }
               cs = (int)(s - (num + 64)) + (3 << 24); // cs is how many tens
               if (pr) {
                  *s++ = stbsp__period;
                  tz = pr;
               }
            } else {
               // handle xxxxx.xxxx000*000
               n = 0;
               for (;;) {
                  if ((fl & STBSP__TRIPLET_COMMA) && (++cs == 4)) {
                     cs = 0;
                     *s++ = stbsp__comma;
                  } else {
                     *s++ = sn[n];
                     ++n;
                     if (n >= (stbsp__uint32)dp)
                        break;
                  }
               }
               cs = (int)(s - (num + 64)) + (3 << 24); // cs is how many tens
               if (pr)
                  *s++ = stbsp__period;
               if ((l - dp) > (stbsp__uint32)pr)
                  l = pr + dp;
               while (n < l) {
                  *s++ = sn[n];
                  ++n;
               }
               tz = pr - (l - dp);
            }
         }
         pr = 0;

         // handle k,m,g,t
         if (fl & STBSP__METRIC_SUFFIX) {
            char idx;
            idx = 1;
            if (fl & STBSP__METRIC_NOSPACE)
               idx = 0;
            tail[0] = idx;
            tail[1] = ' ';
            {
               if (fl >> 24) { // SI kilo is 'k', JEDEC and SI kibits are 'K'.
                  if (fl & STBSP__METRIC_1024)
                     tail[idx + 1] = "_KMGT"[fl >> 24];
                  else
                     tail[idx + 1] = "_kMGT"[fl >> 24];
                  idx++;
                  // If printing kibits and not in jedec, add the 'i'.
                  if (fl & STBSP__METRIC_1024 && !(fl & STBSP__METRIC_JEDEC)) {
                     tail[idx + 1] = 'i';
                     idx++;
                  }
                  tail[0] = idx;
               }
            }
         };

      flt_lead:
         // get the length that we copied
         l = (stbsp__uint32)(s - (num + 64));
         s = num + 64;
         goto scopy;
#endif

      case 'B': // upper binary
      case 'b': // lower binary
         h = (f[0] == 'B') ? hexu : hex;
         lead[0] = 0;
         if (fl & STBSP__LEADING_0X) {
            lead[0] = 2;
            lead[1] = '0';
            lead[2] = h[0xb];
         }
         l = (8 << 4) | (1 << 8);
         goto radixnum;

      case 'o': // octal
         h = hexu;
         lead[0] = 0;
         if (fl & STBSP__LEADING_0X) {
            lead[0] = 1;
            lead[1] = '0';
         }
         l = (3 << 4) | (3 << 8);
         goto radixnum;

      case 'p': // pointer
         fl |= (sizeof(void *) == 8) ? STBSP__INTMAX : 0;
         pr = sizeof(void *) * 2;
         fl &= ~STBSP__LEADINGZERO; // 'p' only prints the pointer with zeros
                                    // fall through - to X

      case 'X': // upper hex
      case 'x': // lower hex
         h = (f[0] == 'X') ? hexu : hex;
         l = (4 << 4) | (4 << 8);
         lead[0] = 0;
         if (fl & STBSP__LEADING_0X) {
            lead[0] = 2;
            lead[1] = '0';
            lead[2] = h[16];
         }
      radixnum:
         // get the number
         if (fl & STBSP__INTMAX)
            n64 = va_arg(va, stbsp__uint64);
         else
            n64 = va_arg(va, stbsp__uint32);

         s = num + STBSP__NUMSZ;
         dp = 0;
         // clear tail, and clear leading if value is zero
         tail[0] = 0;
         if (n64 == 0) {
            lead[0] = 0;
            if (pr == 0) {
               l = 0;
               cs = 0;
               goto scopy;
            }
         }
         // convert to string
         for (;;) {
            *--s = h[n64 & ((1 << (l >> 8)) - 1)];
            n64 >>= (l >> 8);
            if (!((n64) || ((stbsp__int32)((num + STBSP__NUMSZ) - s) < pr)))
               break;
            if (fl & STBSP__TRIPLET_COMMA) {
               ++l;
               if ((l & 15) == ((l >> 4) & 15)) {
                  l &= ~15;
                  *--s = stbsp__comma;
               }
            }
         };
         // get the tens and the comma pos
         cs = (stbsp__uint32)((num + STBSP__NUMSZ) - s) + ((((l >> 4) & 15)) << 24);
         // get the length that we copied
         l = (stbsp__uint32)((num + STBSP__NUMSZ) - s);
         // copy it
         goto scopy;

      case 'u': // unsigned
      case 'i':
      case 'd': // integer
         // get the integer and abs it
         if (fl & STBSP__INTMAX) {
            stbsp__int64 i64 = va_arg(va, stbsp__int64);
            n64 = (stbsp__uint64)i64;
            if ((f[0] != 'u') && (i64 < 0)) {
               n64 = (stbsp__uint64)-i64;
               fl |= STBSP__NEGATIVE;
            }
         } else {
            stbsp__int32 i = va_arg(va, stbsp__int32);
            n64 = (stbsp__uint32)i;
            if ((f[0] != 'u') && (i < 0)) {
               n64 = (stbsp__uint32)-i;
               fl |= STBSP__NEGATIVE;
            }
         }

#ifndef STB_SPRINTF_NOFLOAT
         if (fl & STBSP__METRIC_SUFFIX) {
            if (n64 < 1024)
               pr = 0;
            else if (pr == -1)
               pr = 1;
            fv = (double)(stbsp__int64)n64;
            goto doafloat;
         }
#endif

         // convert to string
         s = num + STBSP__NUMSZ;
         l = 0;

         for (;;) {
            // do in 32-bit chunks (avoid lots of 64-bit divides even with constant denominators)
            char *o = s - 8;
            if (n64 >= 100000000) {
               n = (stbsp__uint32)(n64 % 100000000);
               n64 /= 100000000;
            } else {
               n = (stbsp__uint32)n64;
               n64 = 0;
            }
            if ((fl & STBSP__TRIPLET_COMMA) == 0) {
               do {
                  s -= 2;
                  *(stbsp__uint16 *)s = *(stbsp__uint16 *)&stbsp__digitpair.pair[(n % 100) * 2];
                  n /= 100;
               } while (n);
            }
            while (n) {
               if ((fl & STBSP__TRIPLET_COMMA) && (l++ == 3)) {
                  l = 0;
                  *--s = stbsp__comma;
                  --o;
               } else {
                  *--s = (char)(n % 10) + '0';
                  n /= 10;
               }
            }
            if (n64 == 0) {
               if ((s[0] == '0') && (s != (num + STBSP__NUMSZ)))
                  ++s;
               break;
            }
            while (s != o)
               if ((fl & STBSP__TRIPLET_COMMA) && (l++ == 3)) {
                  l = 0;
                  *--s = stbsp__comma;
                  --o;
               } else {
                  *--s = '0';
               }
         }

         tail[0] = 0;
         stbsp__lead_sign(fl, lead);

         // get the length that we copied
         l = (stbsp__uint32)((num + STBSP__NUMSZ) - s);
         if (l == 0) {
            *--s = '0';
            l = 1;
         }
         cs = l + (3 << 24);
         if (pr < 0)
            pr = 0;

      scopy:
         // get fw=leading/trailing space, pr=leading zeros
         if (pr < (stbsp__int32)l)
            pr = l;
         n = pr + lead[0] + tail[0] + tz;
         if (fw < (stbsp__int32)n)
            fw = n;
         fw -= n;
         pr -= l;

         // handle right justify and leading zeros
         if ((fl & STBSP__LEFTJUST) == 0) {
            if (fl & STBSP__LEADINGZERO) // if leading zeros, everything is in pr
            {
               pr = (fw > pr) ? fw : pr;
               fw = 0;
            } else {
               fl &= ~STBSP__TRIPLET_COMMA; // if no leading zeros, then no commas
            }
         }

         // copy the spaces and/or zeros
         if (fw + pr) {
            stbsp__int32 i;
            stbsp__uint32 c;

            // copy leading spaces (or when doing %8.4d stuff)
            if ((fl & STBSP__LEFTJUST) == 0)
               while (fw > 0) {
                  stbsp__cb_buf_clamp(i, fw);
                  fw -= i;
                  while (i) {
                     if ((((stbsp__uintptr)bf) & 3) == 0)
                        break;
                     *bf++ = ' ';
                     --i;
                  }
                  while (i >= 4) {
                     *(stbsp__uint32 *)bf = 0x20202020;
                     bf += 4;
                     i -= 4;
                  }
                  while (i) {
                     *bf++ = ' ';
                     --i;
                  }
                  stbsp__chk_cb_buf(1);
               }

            // copy leader
            sn = lead + 1;
            while (lead[0]) {
               stbsp__cb_buf_clamp(i, lead[0]);
               lead[0] -= (char)i;
               while (i) {
                  *bf++ = *sn++;
                  --i;
               }
               stbsp__chk_cb_buf(1);
            }

            // copy leading zeros
            c = cs >> 24;
            cs &= 0xffffff;
            cs = (fl & STBSP__TRIPLET_COMMA) ? ((stbsp__uint32)(c - ((pr + cs) % (c + 1)))) : 0;
            while (pr > 0) {
               stbsp__cb_buf_clamp(i, pr);
               pr -= i;
               if ((fl & STBSP__TRIPLET_COMMA) == 0) {
                  while (i) {
                     if ((((stbsp__uintptr)bf) & 3) == 0)
                        break;
                     *bf++ = '0';
                     --i;
                  }
                  while (i >= 4) {
                     *(stbsp__uint32 *)bf = 0x30303030;
                     bf += 4;
                     i -= 4;
                  }
               }
               while (i) {
                  if ((fl & STBSP__TRIPLET_COMMA) && (cs++ == c)) {
                     cs = 0;
                     *bf++ = stbsp__comma;
                  } else
                     *bf++ = '0';
                  --i;
               }
               stbsp__chk_cb_buf(1);
            }
         }

         // copy leader if there is still one
         sn = lead + 1;
         while (lead[0]) {
            stbsp__int32 i;
            stbsp__cb_buf_clamp(i, lead[0]);
            lead[0] -= (char)i;
            while (i) {
               *bf++ = *sn++;
               --i;
            }
            stbsp__chk_cb_buf(1);
         }

         // copy the string
         n = l;
         while (n) {
            stbsp__int32 i;
            stbsp__cb_buf_clamp(i, n);
            n -= i;
            STBSP__UNALIGNED(while (i >= 4) {
               *(stbsp__uint32 volatile *)bf = *(stbsp__uint32 volatile *)s;
               bf += 4;
               s += 4;
               i -= 4;
            })
            while (i) {
               *bf++ = *s++;
               --i;
            }
            stbsp__chk_cb_buf(1);
         }

         // copy trailing zeros
         while (tz) {
            stbsp__int32 i;
            stbsp__cb_buf_clamp(i, tz);
            tz -= i;
            while (i) {
               if ((((stbsp__uintptr)bf) & 3) == 0)
                  break;
               *bf++ = '0';
               --i;
            }
            while (i >= 4) {
               *(stbsp__uint32 *)bf = 0x30303030;
               bf += 4;
               i -= 4;
            }
            while (i) {
               *bf++ = '0';
               --i;
            }
            stbsp__chk_cb_buf(1);
         }

         // copy tail if there is one
         sn = tail + 1;
         while (tail[0]) {
            stbsp__int32 i;
            stbsp__cb_buf_clamp(i, tail[0]);
            tail[0] -= (char)i;
            while (i) {
               *bf++ = *sn++;
               --i;
            }
            stbsp__chk_cb_buf(1);
         }

         // handle the left justify
         if (fl & STBSP__LEFTJUST)
            if (fw > 0) {
               while (fw) {
                  stbsp__int32 i;
                  stbsp__cb_buf_clamp(i, fw);
                  fw -= i;
                  while (i) {
                     if ((((stbsp__uintptr)bf) & 3) == 0)
                        break;
                     *bf++ = ' ';
                     --i;
                  }
                  while (i >= 4) {
                     *(stbsp__uint32 *)bf = 0x20202020;
                     bf += 4;
                     i -= 4;
                  }
                  while (i--)
                     *bf++ = ' ';
                  stbsp__chk_cb_buf(1);
               }
            }
         break;

      default: // unknown, just copy code
         s = num + STBSP__NUMSZ - 1;
         *s = f[0];
         l = 1;
         fw = fl = 0;
         lead[0] = 0;
         tail[0] = 0;
         pr = 0;
         dp = 0;
         cs = 0;
         goto scopy;
      }
      ++f;
   }
endfmt:

   if (!callback)
      *bf = 0;
   else
      stbsp__flush_cb();

done:
   return tlen + (int)(bf - buf);
}

// cleanup
#undef STBSP__LEFTJUST
#undef STBSP__LEADINGPLUS
#undef STBSP__LEADINGSPACE
#undef STBSP__LEADING_0X
#undef STBSP__LEADINGZERO
#undef STBSP__INTMAX
#undef STBSP__TRIPLET_COMMA
#undef STBSP__NEGATIVE
#undef STBSP__METRIC_SUFFIX
#undef STBSP__NUMSZ
#undef stbsp__chk_cb_bufL
#undef stbsp__chk_cb_buf
#undef stbsp__flush_cb
#undef stbsp__cb_buf_clamp

// ============================================================================
//   wrapper functions

STBSP__PUBLICDEF int STB_SPRINTF_DECORATE(sprintf)(char *buf, char const *fmt, ...)
{
   int result;
   va_list va;
   va_start(va, fmt);
   result = STB_SPRINTF_DECORATE(vsprintfcb)(0, 0, buf, fmt, va);
   va_end(va);
   return result;
}

typedef struct stbsp__context {
   char *buf;
   int count;
   int length;
   char tmp[STB_SPRINTF_MIN];
} stbsp__context;

static char *stbsp__clamp_callback(const char *buf, void *user, int len)
{
   stbsp__context *c = (stbsp__context *)user;
   c->length += len;

   if (len > c->count)
      len = c->count;

   if (len) {
      if (buf != c->buf) {
         const char *s, *se;
         char *d;
         d = c->buf;
         s = buf;
         se = buf + len;
         do {
            *d++ = *s++;
         } while (s < se);
      }
      c->buf += len;
      c->count -= len;
   }

   if (c->count <= 0)
      return c->tmp;
   return (c->count >= STB_SPRINTF_MIN) ? c->buf : c->tmp; // go direct into buffer if you can
}

static char * stbsp__count_clamp_callback( const char * buf, void * user, int len )
{
   stbsp__context * c = (stbsp__context*)user;
   (void) sizeof(buf);

   c->length += len;
   return c->tmp; // go direct into buffer if you can
}

STBSP__PUBLICDEF int STB_SPRINTF_DECORATE( vsnprintf )( char * buf, int count, char const * fmt, va_list va )
{
   stbsp__context c;

   if ( (count == 0) && !buf )
   {
      c.length = 0;

      STB_SPRINTF_DECORATE( vsprintfcb )( stbsp__count_clamp_callback, &c, c.tmp, fmt, va );
   }
   else
   {
      int l;

      c.buf = buf;
      c.count = count;
      c.length = 0;

      STB_SPRINTF_DECORATE( vsprintfcb )( stbsp__clamp_callback, &c, stbsp__clamp_callback(0,&c,0), fmt, va );

      // zero-terminate
      l = (int)( c.buf - buf );
      if ( l >= count ) // should never be greater, only equal (or less) than count
         l = count - 1;
      buf[l] = 0;
   }

   return c.length;
}

STBSP__PUBLICDEF int STB_SPRINTF_DECORATE(snprintf)(char *buf, int count, char const *fmt, ...)
{
   int result;
   va_list va;
   va_start(va, fmt);

   result = STB_SPRINTF_DECORATE(vsnprintf)(buf, count, fmt, va);
   va_end(va);

   return result;
}

STBSP__PUBLICDEF int STB_SPRINTF_DECORATE(vsprintf)(char *buf, char const *fmt, va_list va)
{
   return STB_SPRINTF_DECORATE(vsprintfcb)(0, 0, buf, fmt, va);
}

// =======================================================================
//   low level float utility functions

#ifndef STB_SPRINTF_NOFLOAT

// copies d to bits w/ strict aliasing (this compiles to nothing on /Ox)
#define STBSP__COPYFP(dest, src)                   \
   {                                               \
      int cn;                                      \
      for (cn = 0; cn < 8; cn++)                   \
         ((char *)&dest)[cn] = ((char *)&src)[cn]; \
   }

// get float info
static stbsp__int32 stbsp__real_to_parts(stbsp__int64 *bits, stbsp__int32 *expo, double value)
{
   double d;
   stbsp__int64 b = 0;

   // load value and round at the frac_digits
   d = value;

   STBSP__COPYFP(b, d);

   *bits = b & ((((stbsp__uint64)1) << 52) - 1);
   *expo = (stbsp__int32)(((b >> 52) & 2047) - 1023);

   return (stbsp__int32)((stbsp__uint64) b >> 63);
}

static double const stbsp__bot[23] = {
   1e+000, 1e+001, 1e+002, 1e+003, 1e+004, 1e+005, 1e+006, 1e+007, 1e+008, 1e+009, 1e+010, 1e+011,
   1e+012, 1e+013, 1e+014, 1e+015, 1e+016, 1e+017, 1e+018, 1e+019, 1e+020, 1e+021, 1e+022
};
static double const stbsp__negbot[22] = {
   1e-001, 1e-002, 1e-003, 1e-004, 1e-005, 1e-006, 1e-007, 1e-008, 1e-009, 1e-010, 1e-011,
   1e-012, 1e-013, 1e-014, 1e-015, 1e-016, 1e-017, 1e-018, 1e-019, 1e-020, 1e-021, 1e-022
};
static double const stbsp__negboterr[22] = {
   -5.551115123125783e-018,  -2.0816681711721684e-019, -2.0816681711721686e-020, -4.7921736023859299e-021, -8.1803053914031305e-022, 4.5251888174113741e-023,
   4.5251888174113739e-024,  -2.0922560830128471e-025, -6.2281591457779853e-026, -3.6432197315497743e-027, 6.0503030718060191e-028,  2.0113352370744385e-029,
   -3.0373745563400371e-030, 1.1806906454401013e-032,  -7.7705399876661076e-032, 2.0902213275965398e-033,  -7.1542424054621921e-034, -7.1542424054621926e-035,
   2.4754073164739869e-036,  5.4846728545790429e-037,  9.2462547772103625e-038,  -4.8596774326570872e-039
};
static double const stbsp__top[13] = {
   1e+023, 1e+046, 1e+069, 1e+092, 1e+115, 1e+138, 1e+161, 1e+184, 1e+207, 1e+230, 1e+253, 1e+276, 1e+299
};
static double const stbsp__negtop[13] = {
   1e-023, 1e-046, 1e-069, 1e-092, 1e-115, 1e-138, 1e-161, 1e-184, 1e-207, 1e-230, 1e-253, 1e-276, 1e-299
};
static double const stbsp__toperr[13] = {
   8388608,
   6.8601809640529717e+028,
   -7.253143638152921e+052,
   -4.3377296974619174e+075,
   -1.5559416129466825e+098,
   -3.2841562489204913e+121,
   -3.7745893248228135e+144,
   -1.7356668416969134e+167,
   -3.8893577551088374e+190,
   -9.9566444326005119e+213,
   6.3641293062232429e+236,
   -5.2069140800249813e+259,
   -5.2504760255204387e+282
};
static double const stbsp__negtoperr[13] = {
   3.9565301985100693e-040,  -2.299904345391321e-063,  3.6506201437945798e-086,  1.1875228833981544e-109,
   -5.0644902316928607e-132, -6.7156837247865426e-155, -2.812077463003139e-178,  -5.7778912386589953e-201,
   7.4997100559334532e-224,  -4.6439668915134491e-247, -6.3691100762962136e-270, -9.436808465446358e-293,
   8.0970921678014997e-317
};

#if defined(_MSC_VER) && (_MSC_VER <= 1200)
static stbsp__uint64 const stbsp__powten[20] = {
   1,
   10,
   100,
   1000,
   10000,
   100000,
   1000000,
   10000000,
   100000000,
   1000000000,
   10000000000,
   100000000000,
   1000000000000,
   10000000000000,
   100000000000000,
   1000000000000000,
   10000000000000000,
   100000000000000000,
   1000000000000000000,
   10000000000000000000U
};
#define stbsp__tento19th ((stbsp__uint64)1000000000000000000)
#else
static stbsp__uint64 const stbsp__powten[20] = {
   1,
   10,
   100,
   1000,
   10000,
   100000,
   1000000,
   10000000,
   100000000,
   1000000000,
   10000000000ULL,
   100000000000ULL,
   1000000000000ULL,
   10000000000000ULL,
   100000000000000ULL,
   1000000000000000ULL,
   10000000000000000ULL,
   100000000000000000ULL,
   1000000000000000000ULL,
   10000000000000000000ULL
};
#define stbsp__tento19th (1000000000000000000ULL)
#endif

#define stbsp__ddmulthi(oh, ol, xh, yh)                            \
   {                                                               \
      double ahi = 0, alo, bhi = 0, blo;                           \
      stbsp__int64 bt;                                             \
      oh = xh * yh;                                                \
      STBSP__COPYFP(bt, xh);                                       \
      bt &= ((~(stbsp__uint64)0) << 27);                           \
      STBSP__COPYFP(ahi, bt);                                      \
      alo = xh - ahi;                                              \
      STBSP__COPYFP(bt, yh);                                       \
      bt &= ((~(stbsp__uint64)0) << 27);                           \
      STBSP__COPYFP(bhi, bt);                                      \
      blo = yh - bhi;                                              \
      ol = ((ahi * bhi - oh) + ahi * blo + alo * bhi) + alo * blo; \
   }

#define stbsp__ddtoS64(ob, xh, xl)          \
   {                                        \
      double ahi = 0, alo, vh, t;           \
      ob = (stbsp__int64)xh;                \
      vh = (double)ob;                      \
      ahi = (xh - vh);                      \
      t = (ahi - xh);                       \
      alo = (xh - (ahi - t)) - (vh + t);    \
      ob += (stbsp__int64)(ahi + alo + xl); \
   }

#define stbsp__ddrenorm(oh, ol) \
   {                            \
      double s;                 \
      s = oh + ol;              \
      ol = ol - (s - oh);       \
      oh = s;                   \
   }

#define stbsp__ddmultlo(oh, ol, xh, xl, yh, yl) ol = ol + (xh * yl + xl * yh);

#define stbsp__ddmultlos(oh, ol, xh, yl) ol = ol + (xh * yl);

static void stbsp__raise_to_power10(double *ohi, double *olo, double d, stbsp__int32 power) // power can be -323 to +350
{
   double ph, pl;
   if ((power >= 0) && (power <= 22)) {
      stbsp__ddmulthi(ph, pl, d, stbsp__bot[power]);
   } else {
      stbsp__int32 e, et, eb;
      double p2h, p2l;

      e = power;
      if (power < 0)
         e = -e;
      et = (e * 0x2c9) >> 14; /* %23 */
      if (et > 13)
         et = 13;
      eb = e - (et * 23);

      ph = d;
      pl = 0.0;
      if (power < 0) {
         if (eb) {
            --eb;
            stbsp__ddmulthi(ph, pl, d, stbsp__negbot[eb]);
            stbsp__ddmultlos(ph, pl, d, stbsp__negboterr[eb]);
         }
         if (et) {
            stbsp__ddrenorm(ph, pl);
            --et;
            stbsp__ddmulthi(p2h, p2l, ph, stbsp__negtop[et]);
            stbsp__ddmultlo(p2h, p2l, ph, pl, stbsp__negtop[et], stbsp__negtoperr[et]);
            ph = p2h;
            pl = p2l;
         }
      } else {
         if (eb) {
            e = eb;
            if (eb > 22)
               eb = 22;
            e -= eb;
            stbsp__ddmulthi(ph, pl, d, stbsp__bot[eb]);
            if (e) {
               stbsp__ddrenorm(ph, pl);
               stbsp__ddmulthi(p2h, p2l, ph, stbsp__bot[e]);
               stbsp__ddmultlos(p2h, p2l, stbsp__bot[e], pl);
               ph = p2h;
               pl = p2l;
            }
         }
         if (et) {
            stbsp__ddrenorm(ph, pl);
            --et;
            stbsp__ddmulthi(p2h, p2l, ph, stbsp__top[et]);
            stbsp__ddmultlo(p2h, p2l, ph, pl, stbsp__top[et], stbsp__toperr[et]);
            ph = p2h;
            pl = p2l;
         }
      }
   }
   stbsp__ddrenorm(ph, pl);
   *ohi = ph;
   *olo = pl;
}

// given a float value, returns the significant bits in bits, and the position of the
//   decimal point in decimal_pos.  +/-INF and NAN are specified by special values
//   returned in the decimal_pos parameter.
// frac_digits is absolute normally, but if you want from first significant digits (got %g and %e), or in 0x80000000
static stbsp__int32 stbsp__real_to_str(char const **start, stbsp__uint32 *len, char *out, stbsp__int32 *decimal_pos, double value, stbsp__uint32 frac_digits)
{
   double d;
   stbsp__int64 bits = 0;
   stbsp__int32 expo, e, ng, tens;

   d = value;
   STBSP__COPYFP(bits, d);
   expo = (stbsp__int32)((bits >> 52) & 2047);
   ng = (stbsp__int32)((stbsp__uint64) bits >> 63);
   if (ng)
      d = -d;

   if (expo == 2047) // is nan or inf?
   {
      *start = (bits & ((((stbsp__uint64)1) << 52) - 1)) ? "NaN" : "Inf";
      *decimal_pos = STBSP__SPECIAL;
      *len = 3;
      return ng;
   }

   if (expo == 0) // is zero or denormal
   {
      if (((stbsp__uint64) bits << 1) == 0) // do zero
      {
         *decimal_pos = 1;
         *start = out;
         out[0] = '0';
         *len = 1;
         return ng;
      }
      // find the right expo for denormals
      {
         stbsp__int64 v = ((stbsp__uint64)1) << 51;
         while ((bits & v) == 0) {
            --expo;
            v >>= 1;
         }
      }
   }

   // find the decimal exponent as well as the decimal bits of the value
   {
      double ph, pl;

      // log10 estimate - very specifically tweaked to hit or undershoot by no more than 1 of log10 of all expos 1..2046
      tens = expo - 1023;
      tens = (tens < 0) ? ((tens * 617) / 2048) : (((tens * 1233) / 4096) + 1);

      // move the significant bits into position and stick them into an int
      stbsp__raise_to_power10(&ph, &pl, d, 18 - tens);

      // get full as much precision from double-double as possible
      stbsp__ddtoS64(bits, ph, pl);

      // check if we undershot
      if (((stbsp__uint64)bits) >= stbsp__tento19th)
         ++tens;
   }

   // now do the rounding in integer land
   frac_digits = (frac_digits & 0x80000000) ? ((frac_digits & 0x7ffffff) + 1) : (tens + frac_digits);
   if ((frac_digits < 24)) {
      stbsp__uint32 dg = 1;
      if ((stbsp__uint64)bits >= stbsp__powten[9])
         dg = 10;
      while ((stbsp__uint64)bits >= stbsp__powten[dg]) {
         ++dg;
         if (dg == 20)
            goto noround;
      }
      if (frac_digits < dg) {
         stbsp__uint64 r;
         // add 0.5 at the right position and round
         e = dg - frac_digits;
         if ((stbsp__uint32)e >= 24)
            goto noround;
         r = stbsp__powten[e];
         bits = bits + (r / 2);
         if ((stbsp__uint64)bits >= stbsp__powten[dg])
            ++tens;
         bits /= r;
      }
   noround:;
   }

   // kill long trailing runs of zeros
   if (bits) {
      stbsp__uint32 n;
      for (;;) {
         if (bits <= 0xffffffff)
            break;
         if (bits % 1000)
            goto donez;
         bits /= 1000;
      }
      n = (stbsp__uint32)bits;
      while ((n % 1000) == 0)
         n /= 1000;
      bits = n;
   donez:;
   }

   // convert to string
   out += 64;
   e = 0;
   for (;;) {
      stbsp__uint32 n;
      char *o = out - 8;
      // do the conversion in chunks of U32s (avoid most 64-bit divides, worth it, constant denomiators be damned)
      if (bits >= 100000000) {
         n = (stbsp__uint32)(bits % 100000000);
         bits /= 100000000;
      } else {
         n = (stbsp__uint32)bits;
         bits = 0;
      }
      while (n) {
         out -= 2;
         *(stbsp__uint16 *)out = *(stbsp__uint16 *)&stbsp__digitpair.pair[(n % 100) * 2];
         n /= 100;
         e += 2;
      }
      if (bits == 0) {
         if ((e) && (out[0] == '0')) {
            ++out;
            --e;
         }
         break;
      }
      while (out != o) {
         *--out = '0';
         ++e;
      }
   }

   *decimal_pos = tens;
   *start = out;
   *len = e;
   return ng;
}

#undef stbsp__ddmulthi
#undef stbsp__ddrenorm
#undef stbsp__ddmultlo
#undef stbsp__ddmultlos
#undef STBSP__SPECIAL
#undef STBSP__COPYFP

#endif // STB_SPRINTF_NOFLOAT

// clean up
#undef stbsp__uint16
#undef stbsp__uint32
#undef stbsp__int32
#undef stbsp__uint64
#undef stbsp__int64
#undef STBSP__UNALIGNED

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2017 Sean Barrett
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/

/* -----------------------------------------------------------------------------------------------------
   DC: End of third-party code
   -----------------------------------------------------------------------------------------------------*/

#endif