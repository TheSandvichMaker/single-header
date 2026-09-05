/*
	MUD is a small data description format designed to be hand-written. It provides
	a JSON-like data model, with the addition of @tag annotations, inspired by Metadesk.

	MUD is distributed as a single header library. Inside one source file, you
	need to include the header while having MUD_IMPL defined.

		#define MUD_IMPL
		#include "mud.h"

	MUD does zero memory allocation, the only parsing API is mud_parse_from_string which
	takes an array of nodes to use as its backing storage. All MUD_STRINGs point back
	into the original source file.

	Basic usage looks like:

		ptrdiff_t file_size;
		char const *file = read_entire_file("my_file.mud", &file_size);

		#define MAX_NODE_COUNT (1 << 16)
		static Mud_Node nodes[MAX_NODE_COUNT];

		Mud_Parser parser;
		Mud_Parse_Result result = mud_parse_from_string(&parser, nodes, MAX_NODE_COUNT, mud_string(file, file_size));

		if (result.error != Mud_Error_none)
		{
			// TODO: Error reporting
			exit(-1):
		}

		Mud_Node *root = result.root;
		for (Mud_EachChild(node, root))
		{
			do_stuff_with_node(node);
		}

	There are three kinds of elements that can appear at the top level of a MUD file:

		// key-value
		my_value = value

		// array
		my_array [
			1
			2
		]
	
		// object
		my_object {
			a = 1
			b = 2
		}

	Members of arrays and objects can be delineated by commas or newlines

		my_array  [ 1, 2 ]
		my_object { a = 1, b = 2 }

	Elements can be tagged to provide an additional dimension of metadata

		@my_tag my_array [ 1, 2 ]

	Tags can receive arguments, which must be a list of key-value pairs
	
		@my_tag(arg1=1, arg2=2) my_array [ 1, 2 ]

	Leading and trailing comments are attached to elements

		// Leading comment
		my_object {
			member1 = value     // Trailing comment 1
			member2 [ 1, 2, 3 ] // Trailing comment 2
		} // Trailing comment 3

	Here, the leading comment and trailing comment 3 are attached to my_object,
	while trailing comments 1 and 2 are attached to member1 and member2

	There are more defines you can use to configure the library, see below for
	details.

	MUD uses nil nodes rather than null pointers to signify a node's non-existence.
	This means you need to use mud_is_nil():

		for (Mud_Node *node = first; !mud_is_nil(node); node = node->next)

		if (!mud_is_nil(node->first_tag))
		{
			printf("My node has a tag!\n");
		}

	This can be disabled by defining MUD_NIL_IS_NULL prior to including mud.h in your
	source file (so wherever you are including it with MUD_IMPL defined)

	MUD is licensed under the MIT license. See bottom of file for more.
*/

//
// Header
//

#ifndef MUD_H
#define MUD_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t Mud_Bool;

// Define to disable nil nodes
// #define MUD_NIL_IS_NULL

// If you're a savage
#if defined(MUD_PREFER_UNSIGNED)
	typedef size_t Mud_Int;
#else
	typedef ptrdiff_t Mud_Int;
#endif

// If you're only using MUD in one translation unit
#if defined(MUD_STATIC)
	#define MUD_API static inline
#else
	#define MUD_API extern
#endif

#define MUD_INLINE static inline

// If you want to use your own string type, you can define MUD_STRING
// and the accompanying accessor macros. MUD is written to deal with
// counted non-owning strings, aka string views / slices, not
// null-terminated strings or strings which own their own memory.
#if !defined(MUD_STRING)
	typedef struct MUD_STRING
	{
		char const *bytes;
		Mud_Int count;
	} MUD_STRING;
	#define MUD_STRING_BYTES(str) (str).bytes
	#define MUD_STRING_BYTES_ASSIGN(str, value) ((str).bytes = (value))
	#define MUD_STRING_COUNT(str) (str).count
	#define MUD_STRING_COUNT_ASSIGN(str, value) ((str).count = (value))
#else
	#if !defined(MUD_STRING_BYTES) || !defined(MUD_STRING_BYTES_ASSIGN) || !defined(MUD_STRING_COUNT) || !defined(MUD_STRING_COUNT_ASSIGN)
		#error If you provide a custom MUD_STRING, you also need to define the MUD_STRING_BYTES, MUD_STRING_COUNT, MUD_STRING_COUNT_ASSIGN and MUD_STRING_BYTES_ASSIGN macros
	#endif
#endif

//
// Types
//

typedef int Mud_Token;
enum
{
	Mud_Token_eof             = -1,
	Mud_Token_invalid         = -2,
	Mud_Token_string          = -3,
	Mud_Token_identifier      = -4,
	Mud_Token_number          = -5,
	Mud_Token_comment         = -6,
};

typedef uint32_t Mud_Node_Flags;
enum
{
	Mud_Node_Flag_is_array             = (1u << 0),
	Mud_Node_Flag_is_object            = (1u << 1),
	Mud_Node_Flag_is_tag               = (1u << 2),
	Mud_Node_Flag_has_children         = (1u << 3),
	Mud_Node_Flag_has_name             = (1u << 4),
	Mud_Node_Flag_has_value            = (1u << 5),
	Mud_Node_Flag_is_root              = (1u << 6),
	Mud_Node_Flag_is_quoted_string     = (1u << 7),
	Mud_Node_Flag_is_backticked_string = (1u << 8),
	Mud_Node_Flag_is_identifier        = (1u << 9),
	Mud_Node_Flag_is_number            = (1u << 10),
	Mud_Node_Flag_number_is_negative   = (1u << 11),
	Mud_Node_Flag_number_is_octal      = (1u << 12),
	Mud_Node_Flag_number_is_hex        = (1u << 13),
	Mud_Node_Flag_number_is_real       = (1u << 14),
	Mud_Node_Flag_number_is_scientific = (1u << 15),
	Mud_Node_Flag_is_true              = (1u << 16),
	Mud_Node_Flag_is_false             = (1u << 17),
	Mud_Node_Flag_is_bool              = Mud_Node_Flag_is_true|Mud_Node_Flag_is_false,
	Mud_Node_Flag_is_nil               = (1u << 18),
};
#define Mud_Node_Flag_invalid ((Mud_Node_Flags)0x80000000u)

typedef enum Mud_Error
{
	Mud_Error_none         = 0,
	Mud_Error_syntax_error = 1,
	Mud_Error_out_of_nodes = 2,
} Mud_Error;

typedef struct Mud_Node
{
	struct Mud_Node *parent;
	struct Mud_Node *next;
	struct Mud_Node *first_child;
	struct Mud_Node *first_tag;

	Mud_Node_Flags flags;

	Mud_Int line;
	Mud_Int col;

	MUD_STRING name;
	MUD_STRING value;
	MUD_STRING value_unquoted;
	MUD_STRING leading_comment;
	MUD_STRING trailing_comment;
} Mud_Node;

typedef struct Mud_Parser
{
	Mud_Node *out_nodes;
	Mud_Int   out_nodes_used;
	Mud_Int   out_node_capacity;

	MUD_STRING source;

	char const *at;
	char const *end;

	Mud_Node *parent;

	char const *line_start;
	Mud_Int line;
	Mud_Int col;

	Mud_Bool insert_comma;

	Mud_Node *leading_comment;
	Mud_Node *line_comment;

	struct
	{
		char const    *start;
		char const    *end;
		char const    *line_start;
		Mud_Int        line;
		Mud_Int        col;
		MUD_STRING     value;
		Mud_Bool       is_newline;
		char           kind;
		Mud_Node_Flags flags;
	} token;

	Mud_Error error;
	Mud_Int   error_line_start;
	Mud_Int   error_line;
	Mud_Int   error_col;

	Mud_Int message_length;
	char    message_buffer[256];
} Mud_Parser;

typedef struct Mud_Parse_Result
{
	Mud_Node   *root;
	Mud_Error   error;
	MUD_STRING  error_message;
	Mud_Int     error_line_start; // byte offset from source
	Mud_Int     error_line;
	Mud_Int     error_col;
	Mud_Int     node_count;
} Mud_Parse_Result;

//
// API
//

#define Mud_EachNode(it, first)   Mud_Node *it = first;                 !mud_is_nil(it); it = it->next
#define Mud_EachChild(it, parent) Mud_Node *it = (parent)->first_child; !mud_is_nil(it); it = it->next
#define Mud_EachTag(it, parent)   Mud_Node *it = (parent)->first_tag;   !mud_is_nil(it); it = it->next

MUD_INLINE MUD_STRING mud_string(char const *bytes, Mud_Int count)
{
	MUD_STRING result;
	MUD_STRING_BYTES_ASSIGN(result, bytes);
	MUD_STRING_COUNT_ASSIGN(result, count);
	return result;
}

#define MUD_TEXT(lit) mud_string("" lit, sizeof(lit) - 1)

MUD_API Mud_Parse_Result mud_parse_from_string(Mud_Parser *parser, Mud_Node *nodes_buffer, Mud_Int nodes_buffer_size, MUD_STRING source);

MUD_API Mud_Node *mud_nil(void);
MUD_API Mud_Bool mud_is_nil(Mud_Node *node);
MUD_API Mud_Node *mud_get_tag(Mud_Node *node, MUD_STRING name);
MUD_API Mud_Bool mud_has_tag(Mud_Node *node, MUD_STRING name);
MUD_API Mud_Node *mud_get_child(Mud_Node *node, MUD_STRING name);
MUD_API Mud_Bool mud_has_child(Mud_Node *node, MUD_STRING name);

#ifdef __cplusplus
} // extern "C"
#endif

#endif

//
// Implementation
//

#if defined(MUD_IMPL)

MUD_INLINE void mud_error(Mud_Parser *p, Mud_Node *node, Mud_Error error_code, MUD_STRING message)
{
	if (p->error != Mud_Error_none) return;

	if (!mud_is_nil(node))
	{
		node->flags |= Mud_Node_Flag_invalid;
	}

	p->error            = error_code;
	p->error_line_start = p->token.line_start - MUD_STRING_BYTES(p->source);
	p->error_line       = p->token.line;
	p->error_col        = p->token.col;

	p->message_length = 0;
	for (Mud_Int i = 0; i < MUD_STRING_COUNT(message); i += 1)
	{
		p->message_buffer[p->message_length++] = MUD_STRING_BYTES(message)[i];
		if (p->message_length + 1 == sizeof(p->message_buffer))
		{
			break;
		}
	}
	p->message_buffer[p->message_length] = 0;
}

MUD_INLINE Mud_Bool mud_keep_parsing(Mud_Parser *p)
{
	return p->error == Mud_Error_none && p->at[0] != '\0' && p->at < p->end;
}

MUD_INLINE Mud_Bool mud_match_keyword(Mud_Parser *p, MUD_STRING keyword)
{
	char const *reset = p->at;
	for (Mud_Int i = 0; i < MUD_STRING_COUNT(keyword); i += 1)
	{
		if (*p->at++ != MUD_STRING_BYTES(keyword)[i])
		{
			p->at = reset;
			return 0;
		}
	}

	return 1;
}

MUD_INLINE Mud_Bool mud_next(Mud_Parser *p)
{
	if (p->at < p->end)
	{
		p->at  += 1;
		p->col += 1;
		return 1;
	}
	return 0;
}

MUD_INLINE Mud_Bool mud_string_match(MUD_STRING a, MUD_STRING b)
{
	if (MUD_STRING_COUNT(a) != MUD_STRING_COUNT(b)) return 0;
	for (Mud_Int i = 0; i < MUD_STRING_COUNT(a); i += 1)
	{
		if (MUD_STRING_BYTES(a)[i] != MUD_STRING_BYTES(b)[i]) return 0;
	}
	return 1;
}

MUD_INLINE Mud_Bool mud_at_end(Mud_Parser *p)
{
	return p->at == p->end;
}

MUD_INLINE Mud_Bool mud_char_compatible_with_base(char c, Mud_Node_Flags flags)
{
	Mud_Bool is_octal = !!(flags & Mud_Node_Flag_number_is_octal);
	Mud_Bool is_hex   = !!(flags & Mud_Node_Flag_number_is_hex);

	Mud_Bool result = 0;
	if (is_hex && ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')))
	{
		result = 1;
	}
	else if (!is_octal && (c == '8' || c == '9'))
	{
		result = 1;
	}
	else if (c >= '0' && c <= '7')
	{
		result = 1;
	}
	
	return result;
}

MUD_INLINE Mud_Bool mud_char_is_digit(char c)
{
	return c >= '0' && c <= '9';
}

MUD_INLINE void mud_next_token(Mud_Parser *p)
{
	char const *start      = p->at;
	char const *end        = p->end;
	char const *line_start = p->line_start;

	Mud_Int line = 1;
	Mud_Int col  = 0;

	p->token.kind         = Mud_Token_invalid;
	p->token.is_newline   = 0;

	Mud_Node_Flags flags = 0;

	while (!mud_at_end(p))
	{
		start      = p->at;
		line       = p->line;
		col        = p->col;
		line_start = p->line_start;

		switch (p->at[0])
		{
			// EOF
			case '\0':
			{
				start = p->at;
				if (p->insert_comma)
				{
					p->insert_comma = 0;
					p->token.kind = ',';
					mud_next(p);
					goto done;
				}
				else
				{
					p->token.kind = Mud_Token_eof;
					mud_next(p);
					goto done;
				}
			} break;

			// Newline
			case '\n':
			{
				mud_next(p);

				p->line_start = p->at;
				p->line += 1;
				p->col   = 0;

				if (p->insert_comma)
				{
					p->insert_comma = 0;
					p->token.kind = ',';
					p->token.is_newline = 1;
					goto done;
				}
			} break;

			// Whitespace
			case ' ': case '\t': case '\r':
			{
				mud_next(p);
			} break;

			// Line continue
			case '\\':
			{
				mud_next(p);
				p->insert_comma = 0;
			} break;

			// Number
			case '-':
			case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7':
			case '8': case '9':
			{
				// TODO(daniel): Detect malformed numbers!

				p->token.kind = Mud_Token_number;
				flags |= Mud_Node_Flag_is_number;

				if (p->at[0] == '-')
				{
					flags |= Mud_Node_Flag_number_is_negative;
					mud_next(p);
				}

				if (p->at[0] == '0')
				{
					mud_next(p);

					if (p->at[0] != '.' && p->at[0] != ',')
					{
						if (p->at[0] == 'x' || p->at[0] == 'X' ||
							p->at[0] == 'b' || p->at[0] == 'B')
						{
							flags |= Mud_Node_Flag_number_is_hex;
							mud_next(p);
						}
						else
						{
							flags |= Mud_Node_Flag_number_is_octal;
						}
					}
				}

				while (mud_char_compatible_with_base(p->at[0], flags))
				{
					mud_next(p);
				}

				if ((flags & (Mud_Node_Flag_number_is_octal|Mud_Node_Flag_number_is_hex)) == 0)
				{
					if (p->at[0] == '.')
					{
						mud_next(p);

						while (mud_char_is_digit(p->at[0]))
						{
							mud_next(p);
						}
					}

					if (p->at[0] == 'e' || p->at[0] == 'E')
					{
						mud_next(p);

						if (p->at[0] == '-' || p->at[0] == '+')
						{
							mud_next(p);
						}

						while (mud_char_is_digit(p->at[0]))
						{
							mud_next(p);
						}
					}
				}

				goto done;
			} break;

			// Comments
			case '/':
			{
				p->token.kind = Mud_Token_comment;

				mud_next(p);

				if (p->at[0] == '/')
				{
					mud_next(p);

					while (p->at < end && p->at[0] != '\n')
					{
						mud_next(p);
					}
				}
				else if (p->at[0] == '*')
				{
					mud_next(p);

					while (p->at + 1 < end && !(p->at[0] == '*' && p->at[1] == '/'))
					{
						if (p->at[0] == '\n')
						{
							mud_next(p);
							p->line_start = p->at;
							p->line += 1;
						}
						else
						{
							mud_next(p);
						}
					}
					mud_next(p);
					mud_next(p);

					if (p->at >= end)
					{
						mud_error(p, NULL, Mud_Error_syntax_error, MUD_TEXT("unexpected end of file in block comment"));
						p->token.kind = Mud_Token_eof;
						goto done;
					}
				}

				goto done;
			} break;

			// Strings
			case '`': case '"':
			{
				p->token.kind = Mud_Token_string;

				char delimiter = p->at[0];
				mud_next(p);

				/**/ if (delimiter == '"') flags |= Mud_Node_Flag_is_quoted_string;
				else if (delimiter == '`') flags |= Mud_Node_Flag_is_backticked_string;

				while (p->at < end && p->at[0] != delimiter)
				{
					// TODO(daniel): Do I need other escape handling?
					if (p->at[0] == '\\' && p->at + 1 < end && p->at[1] == '"')
					{
						mud_next(p);
					}

					mud_next(p);
				}

				if (p->at[0] != delimiter)
				{
					mud_error(p, NULL, Mud_Error_syntax_error, MUD_TEXT("unexpected end of file in string comment"));
					p->token.kind = Mud_Token_eof;
				}

				mud_next(p);

				goto done;
			} break;

			// Identifiers and keywords (which are just identifiers with extra flags)
			default:
			{
				if (mud_match_keyword(p, MUD_TEXT("true")))
				{
					p->token.kind = Mud_Token_identifier;
					flags |= Mud_Node_Flag_is_identifier|Mud_Node_Flag_is_true;
					goto done;
				}
				else if (mud_match_keyword(p, MUD_TEXT("false")))
				{
					p->token.kind = Mud_Token_identifier;
					flags |= Mud_Node_Flag_is_identifier|Mud_Node_Flag_is_false;
					goto done;
				}
				else if (mud_match_keyword(p, MUD_TEXT("nil")))
				{
					p->token.kind = Mud_Token_identifier;
					flags |= Mud_Node_Flag_is_identifier|Mud_Node_Flag_is_nil;
					goto done;
				}
				else if ((p->at[0] >= 'a' && p->at[0] <= 'z') || (p->at[0] >= 'A' && p->at[0] <= 'Z') || p->at[0] == '_')
				{
					mud_next(p);

					p->token.kind = Mud_Token_identifier;
					flags |= Mud_Node_Flag_is_identifier;

					while (p->at < end && ((p->at[0] >= 'a' && p->at[0] <= 'z') || (p->at[0] >= 'A' && p->at[0] <= 'Z') || (p->at[0] >= '0' && p->at[0] <= '9') || p->at[0] == '_'))
					{
						mud_next(p);
					}

					goto done;
				}
				else
				{
					// Single char tokens
					p->token.kind = p->at[0];
					mud_next(p);
					goto done;
				}
			} break;
		}
	}

done:
	p->token.start       = start;
	p->token.end         = p->at;
	p->token.value       = mud_string(p->token.start, (Mud_Int)(p->token.end - p->token.start));
	p->token.line_start  = line_start;
	p->token.line        = line;
	p->token.col         = col;
	p->token.flags       = flags;

	switch (p->token.kind)
	{
		case Mud_Token_comment:
		{
			// leave comma insertion state untouched
		} break;

		case Mud_Token_identifier:
		case Mud_Token_number:
		case Mud_Token_string:
		case ']': case '}':
		case '=':
		{
			p->insert_comma = 1;
		} break;

		default:
		{
			p->insert_comma = 0;
		} break;
	}
}

MUD_INLINE Mud_Bool mud_peek_token(Mud_Parser *p, Mud_Token kind)
{
	return p->token.kind == kind;
}

MUD_INLINE Mud_Bool mud_eat_token(Mud_Parser *p, Mud_Token kind)
{
	Mud_Bool result = 0;

	if (mud_peek_token(p, kind))
	{
		result = 1;
		mud_next_token(p);
	}

	return result;
}

MUD_INLINE Mud_Node *mud_allocate_node(Mud_Parser *p)
{
	if (p->out_nodes_used >= p->out_node_capacity)
	{
		return mud_nil();
	}

	Mud_Node *result = &p->out_nodes[p->out_nodes_used++];
	result->parent           = p->parent;
	result->next             = mud_nil();
	result->first_child      = mud_nil();
	result->first_tag        = mud_nil();
	result->flags            = 0;
	result->line             = 1;
	result->col              = 0;
	result->name             = mud_string(NULL, 0);
	result->value            = mud_string(NULL, 0);
	result->value_unquoted   = mud_string(NULL, 0);
	result->leading_comment  = mud_string(NULL, 0);
	result->trailing_comment = mud_string(NULL, 0);

	return result;
}

MUD_INLINE void mud_set_name(Mud_Node *node, MUD_STRING name)
{
	node->name = name;

	if (MUD_STRING_COUNT(node->name) > 0)
	{
		node->flags |= Mud_Node_Flag_has_name;
	}
}

MUD_INLINE void mud_set_value(Mud_Node *node, MUD_STRING value)
{
	node->value          = value;
	node->value_unquoted = value;

	if (node->flags & (Mud_Node_Flag_is_quoted_string|Mud_Node_Flag_is_backticked_string))
	{
		MUD_STRING_BYTES_ASSIGN(node->value_unquoted, MUD_STRING_BYTES(node->value) + 1);
		MUD_STRING_COUNT_ASSIGN(node->value_unquoted, MUD_STRING_COUNT(node->value) - 1);
	}

	if (MUD_STRING_COUNT(node->value) > 0)
	{
		node->flags |= Mud_Node_Flag_has_value;
	}
}

MUD_INLINE void mud_add_node(Mud_Node **first, Mud_Node **last, Mud_Node *node)
{
	if (mud_is_nil(*first))
	{
		*first = node;
		*last  = node;
	}
	else
	{
		(*last)->next = node;
		*last = node;
	}
}

MUD_INLINE MUD_STRING mud_parse_comment(Mud_Parser *p)
{
	MUD_STRING comment = mud_string(NULL, 0);

	if (mud_peek_token(p, Mud_Token_comment))
	{
		char const *start = p->token.start;
		char const *end   = p->token.end;

		while (mud_peek_token(p, Mud_Token_comment))
		{
			end = p->token.end;
			mud_next_token(p);
		}

		comment = mud_string(start, end - start);
	}

	return comment;
}

MUD_INLINE void mud_parse_object_style_value(Mud_Parser *p, Mud_Node *node, Mud_Token closing)
{
	while (mud_eat_token(p, Mud_Token_comment));

	char const *start = p->token.start;
	char const *end   = p->token.start;

	Mud_Int bracket_depth = 0;
	Mud_Int brace_depth   = 0;
	Mud_Int paren_depth   = 0;

	Mud_Node_Flags flags = 0;

	Mud_Int iter = 0;

	while (mud_keep_parsing(p) && !mud_peek_token(p, Mud_Token_comment))
	{
		if (p->token.kind == '[') bracket_depth += 1;
		if (p->token.kind == '{') brace_depth   += 1;
		if (p->token.kind == '(') paren_depth   += 1;

		if (p->token.kind == ']' && bracket_depth > 0) bracket_depth -= 1;
		if (p->token.kind == '}' && brace_depth   > 0) brace_depth   -= 1;
		if (p->token.kind == ')' && paren_depth   > 0) paren_depth   -= 1;

		if (p->token.kind == ',')
		{
			// If the comma is not nested, or if it was a newline, then break.
			if ((bracket_depth == 0 && brace_depth == 0 && paren_depth == 0) || p->token.is_newline)
			{
				break;
			}
		}

		if (p->token.kind == closing)
		{
			if (bracket_depth == 0 && brace_depth == 0 && paren_depth == 0)
			{
				break;
			}
		}

		iter += 1;

		end   = p->token.end;
		flags = p->token.flags;

		mud_next_token(p);
	}

	// You only get flags if there was only a single token.
	if (iter != 1)
	{
		flags = 0;
	}

	node->flags         |= flags;
	node->value          = mud_string(start, (Mud_Int)(end - start));
	node->value_unquoted = node->value;

	if (node->flags & (Mud_Node_Flag_is_quoted_string|Mud_Node_Flag_is_backticked_string))
	{
		MUD_STRING_BYTES_ASSIGN(node->value_unquoted, MUD_STRING_BYTES(node->value) + 1);
		MUD_STRING_COUNT_ASSIGN(node->value_unquoted, MUD_STRING_COUNT(node->value) - 1);
	}

	if (MUD_STRING_COUNT(node->value) > 0)
	{
		node->flags |= Mud_Node_Flag_has_value;
	}
}

MUD_INLINE void mud_parse_array_style_value(Mud_Parser *p, Mud_Node *node)
{
	while (mud_eat_token(p, Mud_Token_comment));

	node->flags         |= p->token.flags;
	node->value          = p->token.value;
	node->value_unquoted = node->value;

	if (!mud_eat_token(p, Mud_Token_identifier) && !mud_eat_token(p, Mud_Token_number) && !mud_eat_token(p, Mud_Token_string))
	{
		mud_error(p, node, Mud_Error_syntax_error, MUD_TEXT("Array values must be an identifier, number, or string"));
		node->flags |= Mud_Node_Flag_invalid;
	}

	if (node->flags & (Mud_Node_Flag_is_quoted_string|Mud_Node_Flag_is_backticked_string))
	{
		MUD_STRING_BYTES_ASSIGN(node->value_unquoted, MUD_STRING_BYTES(node->value) + 1);
		MUD_STRING_COUNT_ASSIGN(node->value_unquoted, MUD_STRING_COUNT(node->value) - 1);
	}

	if (MUD_STRING_COUNT(node->value) > 0)
	{
		node->flags |= Mud_Node_Flag_has_value;
	}
}

enum
{
	Mud_Parse_Flag_allow_objects             = (1u << 0),
	Mud_Parse_Flag_allow_arrays              = (1u << 1),
	Mud_Parse_Flag_allow_object_style_values = (1u << 2),
	Mud_Parse_Flag_allow_array_style_values  = (1u << 3),
	Mud_Parse_Flag_expect_names              = (1u << 4),
	Mud_Parse_Flag_allow_tags                = (1u << 5),
	Mud_Parse_Group_objects  = Mud_Parse_Flag_allow_tags|Mud_Parse_Flag_allow_objects|Mud_Parse_Flag_allow_arrays|Mud_Parse_Flag_allow_object_style_values|Mud_Parse_Flag_expect_names,
	Mud_Parse_Group_arrays   = Mud_Parse_Flag_allow_tags|Mud_Parse_Flag_allow_objects|Mud_Parse_Flag_allow_arrays|Mud_Parse_Flag_allow_array_style_values,
	Mud_Parse_Group_tag_args = Mud_Parse_Flag_allow_object_style_values|Mud_Parse_Flag_expect_names,
};

MUD_INLINE void mud_parse_element(Mud_Parser *p, Mud_Node *node, Mud_Int parse_flags, Mud_Token end_token)
{
	if (parse_flags == Mud_Parse_Group_objects)
	{
		node->flags |= Mud_Node_Flag_is_object;
	}
	else
	{
		node->flags |= Mud_Node_Flag_is_array;
	}

	Mud_Node *first_child = mud_nil();
	Mud_Node *last_child  = mud_nil();

	while (mud_keep_parsing(p))
	{
		MUD_STRING leading_comment = mud_parse_comment(p);

		// Allow empty lists, and make sure comments before the end token don't cause problems.
		if (mud_eat_token(p, end_token))
		{
			break;
		}

		Mud_Node *child = mud_allocate_node(p);

		p->parent = child;

		Mud_Node *first_tag = mud_nil();
		Mud_Node *last_tag  = mud_nil();

		if (mud_peek_token(p, '@'))
		{
			if (!(parse_flags & Mud_Parse_Flag_allow_tags))
			{
				mud_error(p, child, Mud_Error_syntax_error, MUD_TEXT("Tag is not allowed"));
				goto bail;
			}

			while (mud_eat_token(p, '@'))
			{
				MUD_STRING name = p->token.value;

				if (!mud_eat_token(p, Mud_Token_identifier))
				{
					mud_error(p, child, Mud_Error_syntax_error, MUD_TEXT("Expected identifier after '@'"));
					goto bail;
				}

				while (mud_eat_token(p, Mud_Token_comment));

				Mud_Node *tag = mud_allocate_node(p);
				tag->flags |= Mud_Node_Flag_is_tag;

				mud_set_name(tag, name);

				p->parent = tag;

				if (mud_eat_token(p, '('))
				{
					mud_parse_element(p, tag, Mud_Parse_Group_tag_args, ')');
				}

				if (!mud_is_nil(tag->first_child))
				{
					tag->flags |= Mud_Node_Flag_has_children;
				}

				p->parent = tag->parent;

				// Consume a newline is it if there so that we can attach to nodes on the next line without issue.
				if (p->token.is_newline) mud_next_token(p);
		
				mud_add_node(&first_tag, &last_tag, tag);
			}
		}

		MUD_STRING name = mud_string(NULL, 0);
		
		if (parse_flags & Mud_Parse_Flag_expect_names)
		{
			name = p->token.value;

			if (!mud_eat_token(p, Mud_Token_identifier))
			{
				mud_error(p, node, Mud_Error_syntax_error, MUD_TEXT("Expected identifier"));
				goto bail;
			}
		}

		while (mud_eat_token(p, Mud_Token_comment));

		mud_set_name(child, name);
		child->line            = p->token.line;
		child->col             = p->token.col;
		child->first_tag       = first_tag;
		child->leading_comment = leading_comment;

		/**/ if (mud_eat_token(p, '{'))
		{
			if (!(parse_flags & Mud_Parse_Flag_allow_objects))
			{
				mud_error(p, child, Mud_Error_syntax_error, MUD_TEXT("Object is not allowed")); goto bail;
			}
			mud_parse_element(p, child, Mud_Parse_Group_objects, '}');
		}
		else if (mud_eat_token(p, '['))
		{
			if (!(parse_flags & Mud_Parse_Flag_allow_arrays))
			{
				mud_error(p, child, Mud_Error_syntax_error, MUD_TEXT("Array is not allowed")); goto bail;
			}
			mud_parse_element(p, child, Mud_Parse_Group_arrays, ']');
		}
		else if (mud_eat_token(p, '='))
		{
			if (!(parse_flags & Mud_Parse_Flag_allow_object_style_values))
			{
				mud_error(p, child, Mud_Error_syntax_error, MUD_TEXT("Object-style value is not allowed")); goto bail;
			}
			mud_parse_object_style_value(p, child, end_token);
		}
		else if (parse_flags & Mud_Parse_Flag_allow_array_style_values)
		{
			mud_parse_array_style_value(p, child);
		}
		else
		{
			mud_error(p, child, Mud_Error_syntax_error, MUD_TEXT("Object children must be followed by {, [ or = delineating the start of an object, array, or value respectively"));
			goto bail;
		}

		p->parent = child->parent;

		mud_add_node(&first_child, &last_child, child);

		// Feels a little messy.
		Mud_Bool ate_comma = !p->token.is_newline && mud_eat_token(p, ',');

		if (mud_peek_token(p, Mud_Token_comment))
		{
			child->trailing_comment = mud_parse_comment(p);
		}

		ate_comma |= mud_eat_token(p, ',');

		if (mud_eat_token(p, end_token))
		{
			break;
		}
		else if (!ate_comma)
		{
			mud_error(p, child, Mud_Error_syntax_error, MUD_TEXT("Expected comma"));
			goto bail;
		}
	}

bail:
	node->first_child = first_child;

	if (!mud_is_nil(node->first_child))
	{
		node->flags |= Mud_Node_Flag_has_children;
	}
}

MUD_INLINE void mud_memset(void *mem, uint8_t value, Mud_Int count)
{
	uint8_t *bytes = (uint8_t *)mem;
	for (Mud_Int i = 0; i < count; i += 1)
	{
		bytes[i] = value;
	}
}

Mud_Parse_Result mud_parse_from_string(Mud_Parser *p, Mud_Node *nodes_buffer, Mud_Int nodes_buffer_size, MUD_STRING source)
{
	mud_memset(p, 0, sizeof(*p));
	p->out_nodes = nodes_buffer;
	p->out_node_capacity = nodes_buffer_size;
	p->source = source;
	p->at     = (char const *)MUD_STRING_BYTES(source);
	p->end    = (char const *)MUD_STRING_BYTES(source) + MUD_STRING_COUNT(source);
	p->line   = 1;

	mud_next_token(p);

	Mud_Parse_Result result;
	mud_memset(&result, 0, sizeof(result));

	result.root = mud_allocate_node(p);
	mud_set_name(result.root, MUD_TEXT("root"));

	if (!mud_is_nil(result.root))
	{
		mud_parse_element(p, result.root, Mud_Parse_Group_objects, Mud_Token_eof);
		result.root->flags |= Mud_Node_Flag_is_root;

		result.error            = p->error;
		result.error_line_start = p->error_line_start;
		result.error_line       = p->error_line;
		result.error_col        = p->error_col;
	}
	else
	{
		result.error = Mud_Error_out_of_nodes;
	}

	result.node_count = p->out_nodes_used;
	MUD_STRING_BYTES_ASSIGN(result.error_message, p->message_buffer);
	MUD_STRING_COUNT_ASSIGN(result.error_message, p->message_length);
	return result;
}

Mud_Node *mud_nil(void)
{
#if defined(MUD_NIL_IS_NULL)
	return NULL;
#else
	static Mud_Node _mud_nil;

	if (_mud_nil.parent == 0)
	{
		_mud_nil.parent      = &_mud_nil;
		_mud_nil.next        = &_mud_nil;
		_mud_nil.first_child = &_mud_nil;
		_mud_nil.first_tag   = &_mud_nil;
	}

	return &_mud_nil;
#endif
}

Mud_Bool mud_is_nil(Mud_Node *node)
{
	return node == NULL || node == mud_nil();
}

Mud_Node *mud_get_tag(Mud_Node *node, MUD_STRING name)
{
	for (Mud_EachTag(tag, node))
	{
		if (mud_string_match(tag->name, name))
		{
			return tag;
		}
	}
	return mud_nil();
}

Mud_Bool mud_has_tag(Mud_Node *node, MUD_STRING name)
{
	return !mud_is_nil(mud_get_tag(node, name));
}

Mud_Node *mud_get_child(Mud_Node *node, MUD_STRING name)
{
	for (Mud_EachChild(child, node))
	{
		if (mud_string_match(child->name, name))
		{
			return child;
		}
	}
	return mud_nil();
}

Mud_Bool mud_has_child(Mud_Node *node, MUD_STRING name)
{
	return !mud_is_nil(mud_get_child(node, name));
}

#endif

// MIT License
// 
// Copyright (c) 2026 Daniël Cornelisse
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
