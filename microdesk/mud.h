//
// Header
//

#ifndef MUD_H
#define MUD_H

#include <stdint.h>
#include <stddef.h>

typedef uint8_t Mud_Bool;

#if defined(MUD_PREFER_UNSIGNED)
	typedef size_t Mud_Int;
#else
	typedef ptrdiff_t Mud_Int;
#endif

#if defined(MUD_STATIC)
	#define MUD_API static
#else
	#define MUD_API extern
#endif

#define MUD_INLINE static inline

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

MUD_INLINE MUD_STRING mud_string(char const *bytes, Mud_Int count)
{
	MUD_STRING result;
	MUD_STRING_BYTES_ASSIGN(result, bytes);
	MUD_STRING_COUNT_ASSIGN(result, count);
	return result;
}

#define MUD_TEXT(lit) mud_string("" lit, sizeof(lit) - 1)

typedef enum Mud_Token
{
	Mud_Token_eof             = -1,
	Mud_Token_invalid         = -2,
	Mud_Token_string          = -3,
	Mud_Token_identifier      = -4,
	Mud_Token_number          = -5,
	Mud_Token_comment         = -6,
} Mud_Token;

typedef uint32_t Mud_Node_Flags;
enum
{
	Mud_Node_Flag_is_array             = (1u << 0),
	Mud_Node_Flag_is_object            = (1u << 1),
	Mud_Node_Flag_is_container         = Mud_Node_Flag_is_array|Mud_Node_Flag_is_object,
	Mud_Node_Flag_is_tag               = (1u << 2),
	Mud_Node_Flag_has_name             = (1u << 3),
	Mud_Node_Flag_has_value            = (1u << 4),
	Mud_Node_Flag_is_root              = (1u << 5),
	Mud_Node_Flag_is_quoted_string     = (1u << 6),
	Mud_Node_Flag_is_backticked_string = (1u << 7),
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

	Mud_Int line;
	Mud_Int col;

	Mud_Bool insert_comma;

	Mud_Node *leading_comment;
	Mud_Node *line_comment;

	struct
	{
		char const    *start;
		char const    *end;
		Mud_Int        line;
		Mud_Int        col;
		MUD_STRING     value;
		Mud_Bool       is_newline;
		char           kind;
		Mud_Node_Flags flags;
	} token;

	Mud_Error error;

	Mud_Int message_length;
	char    message_buffer[256];
} Mud_Parser;

typedef struct Mud_Parse_Result
{
	Mud_Error  error;
	MUD_STRING error_message; 
	Mud_Int    node_count;
} Mud_Parse_Result;

MUD_API MUD_STRING mud_token_to_string(Mud_Token tok);
MUD_API Mud_Parse_Result mud_parse_from_string(Mud_Parser *parser, Mud_Node *nodes_buffer, Mud_Int nodes_buffer_size, MUD_STRING source);

#endif

//
// Implementation
//

#if defined(MUD_IMPL)

MUD_INLINE void mud_error(Mud_Parser *p, Mud_Error error_code, MUD_STRING message)
{
	p->error = error_code;
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

MUD_INLINE Mud_Bool mud_at_end(Mud_Parser *p)
{
	return p->at == p->end;
}

MUD_INLINE Mud_Bool mud_char_compatible_with_base(char c, Mud_Node_Flags flags)
{
	Mud_Bool is_octal = !!(flags & Mud_Node_Flag_number_is_octal);
	Mud_Bool is_hex   = !!(flags & Mud_Node_Flag_number_is_hex);

	Mud_Bool result = 0;
	if (is_hex && ((c >= 'a' && c <= 'f') || c >= 'A' && c <= 'F'))
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
	char const *start = p->at;
	char const *end   = p->end;

	Mud_Int line = 1;
	Mud_Int col  = 0;

	p->token.kind         = Mud_Token_invalid;
	p->token.is_newline   = 0;

	Mud_Node_Flags flags = 0;

	while (!mud_at_end(p))
	{
		start = p->at;
		line  = p->line;
		col   = p->col;

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
						if (p->at[0] == '\n') p->line += 1;
						mud_next(p);
					}
					mud_next(p);
					mud_next(p);

					if (p->at >= end)
					{
						mud_error(p, Mud_Error_syntax_error, MUD_TEXT("unexpected end of file in block comment"));
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
					mud_error(p, Mud_Error_syntax_error, MUD_TEXT("unexpected end of file in string comment"));
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
				}
				else if (mud_match_keyword(p, MUD_TEXT("false")))
				{
					p->token.kind = Mud_Token_identifier;
					flags |= Mud_Node_Flag_is_identifier|Mud_Node_Flag_is_false;
				}
				else if (mud_match_keyword(p, MUD_TEXT("nil")))
				{
					p->token.kind = Mud_Token_identifier;
					flags |= Mud_Node_Flag_is_identifier|Mud_Node_Flag_is_nil;
				}
				if ((p->at[0] >= 'a' && p->at[0] <= 'z') || (p->at[0] >= 'A' && p->at[0] <= 'Z') || p->at[0] == '_')
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
	p->token.start = start;
	p->token.end   = p->at;
	p->token.value = mud_string(p->token.start, (Mud_Int)(p->token.end - p->token.start));
	p->token.line  = line;
	p->token.col   = col;
	p->token.flags = flags;

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

MUD_INLINE MUD_STRING mud_parse_equals_style_value(Mud_Parser *p, Mud_Node_Flags *out_flags)
{
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
		if (p->token.kind == ']')
		{
			if (bracket_depth == 0) break;
			bracket_depth -= 1;
		}
		if (p->token.kind == '}')
		{
			if (brace_depth == 0) break;
			brace_depth -= 1;
		}
		if (p->token.kind == ')') paren_depth   -= 1;

		if (p->token.kind == ',')
		{
			// If the comma is not nested, or if it was an inserted comma, then break.
			if ((bracket_depth == 0 && brace_depth == 0 && paren_depth == 0) || p->token.is_newline)
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
	if (iter == 1)
	{
		*out_flags |= flags;
	}

	MUD_STRING result = mud_string(start, (Mud_Int)(end - start));
	return result;
}

MUD_INLINE Mud_Node *mud_parse_list(Mud_Parser *p, Mud_Bool has_names, Mud_Token end_token);

MUD_INLINE Mud_Node *mud_allocate_node(Mud_Parser *p)
{
	if (p->out_nodes_used >= p->out_node_capacity)
	{
		return NULL;
	}

	Mud_Node *result = &p->out_nodes[p->out_nodes_used++];
	return result;
}

Mud_Node *mud_parse_node(Mud_Parser *p, MUD_STRING name, Mud_Bool value_with_equals, Mud_Bool can_have_value)
{
	Mud_Node *result = mud_allocate_node(p);

	if (result == NULL)
	{
		mud_error(p, Mud_Error_out_of_nodes, MUD_TEXT("Ran out of nodes!"));
		return NULL;
	}

	result->parent = p->parent;
	p->parent = result;

	Mud_Int line = p->token.line;
	Mud_Int col  = p->token.col;

	Mud_Node_Flags flags = 0;

	MUD_STRING value          = MUD_TEXT("");
	MUD_STRING value_unquoted = MUD_TEXT("");

	Mud_Node *first_child = NULL;
	if (mud_eat_token(p, '{'))
	{
		value = p->token.value;
		flags |= Mud_Node_Flag_is_object;
		first_child = mud_parse_list(p, 1, '}');
	}
	else if (mud_eat_token(p, '['))
	{
		value = p->token.value;
		flags |= Mud_Node_Flag_is_array;
		first_child = mud_parse_list(p, 0, ']');
	}
	else if (can_have_value)
	{
		if (value_with_equals && !mud_eat_token(p, '='))
		{
			mud_error(p, Mud_Error_syntax_error, MUD_TEXT("Expected {, [ or ="));
			goto bail;
		}

		if (value_with_equals)
		{
			value = mud_parse_equals_style_value(p, &flags);
		}
		else if (p->token.kind == Mud_Token_identifier || p->token.kind == Mud_Token_string || p->token.kind == Mud_Token_number)
		{
			value  = p->token.value;
			flags |= p->token.flags;
			mud_next_token(p);
		}
		else
		{
			mud_error(p, Mud_Error_syntax_error, MUD_TEXT("Values in arrays must be identifiers, strings, or numbers"));
		}

		if (flags & (Mud_Node_Flag_is_quoted_string|Mud_Node_Flag_is_backticked_string))
		{
			MUD_STRING_BYTES_ASSIGN(value_unquoted, MUD_STRING_BYTES(value) + 1);
			MUD_STRING_COUNT(value_unquoted) = MUD_STRING_COUNT(value) - 1;
			if (MUD_STRING_COUNT(value) < 0)
			{
				MUD_STRING_BYTES_ASSIGN(value_unquoted, NULL);
				MUD_STRING_COUNT(value_unquoted) = 0;
			}
		}

		flags |= Mud_Node_Flag_has_value;
	}

	p->parent = result->parent;

	if (MUD_STRING_COUNT(name) > 0)
	{
		flags |= Mud_Node_Flag_has_name;
	}

	if (MUD_STRING_COUNT(value_unquoted) == 0)
	{
		value_unquoted = value;
	}

	result->first_child      = first_child;
	result->flags            = flags;
	result->line             = line;
	result->col              = col;
	result->name             = name;
	result->value            = value;
	result->value_unquoted   = value_unquoted;
	result->leading_comment  = MUD_TEXT("");
	result->trailing_comment = MUD_TEXT("");

bail:
	return result;
}

MUD_INLINE Mud_Bool mud_skip_newline(Mud_Parser *p)
{
	if (p->token.is_newline)
	{
		mud_next_token(p);
		return 1;
	}

	return 0;
}

MUD_INLINE MUD_STRING mud_parse_comment(Mud_Parser *p)
{
	MUD_STRING comment = MUD_TEXT("");

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

Mud_Node *mud_parse_list(Mud_Parser *p, Mud_Bool has_names, Mud_Token end_token)
{
	Mud_Node *first_child   = NULL;
	Mud_Node *last_child    = NULL;
	Mud_Node *first_tag     = NULL;
	Mud_Node *last_tag      = NULL;

	while (mud_keep_parsing(p))
	{
		MUD_STRING leading_comment = mud_parse_comment(p);

		while (mud_eat_token(p, '@'))
		{
			MUD_STRING name = p->token.value;

			if (!mud_eat_token(p, Mud_Token_identifier))
			{
				mud_error(p, Mud_Error_syntax_error, MUD_TEXT("Expected identifier after @"));
				goto bail;
			}

			Mud_Node *tag = mud_parse_node(p, name, 0, 0);

			if (tag != NULL)
			{
				tag->flags |= Mud_Node_Flag_is_tag;
				if (first_tag == NULL)
				{
					first_tag = last_tag = tag;
				}
				else
				{
					last_tag->next = tag;
					last_tag = tag;
				}
			}

			mud_skip_newline(p);
		}

		// Allow empty lists
		if (!mud_peek_token(p, end_token))
		{
			MUD_STRING name = MUD_TEXT("");

			if (has_names)
			{
				name = p->token.value;

				if (!mud_eat_token(p, Mud_Token_identifier))
				{
					mud_error(p, Mud_Error_syntax_error, MUD_TEXT("Expected identifier"));
					goto bail;
				}

				mud_skip_newline(p);
			}

			Mud_Node *child = mud_parse_node(p, name, has_names, 1);

			if (child != NULL)
			{
				child->first_tag = first_tag;

				first_tag = NULL;
				last_tag  = NULL;

				child->leading_comment = leading_comment;
				leading_comment = MUD_TEXT("");

				if (first_child == NULL)
				{
					first_child = last_child = child;
				}
				else
				{
					last_child->next = child;
					last_child = child;
				}
			}
			else
			{
				goto bail;
			}
		}

		Mud_Bool ate_comma = 0;

		while (!p->token.is_newline && mud_eat_token(p, ',')) ate_comma = 1;

		MUD_STRING trailing_comment = mud_parse_comment(p);

		if (last_child != NULL)
		{
			last_child->trailing_comment = trailing_comment;
		}

		while (mud_eat_token(p, ','))
		{
			ate_comma = 1;
		}

		if (!mud_keep_parsing(p) || mud_eat_token(p, end_token))
		{
			break;
		}
		else if (!ate_comma)
		{
			mud_error(p, Mud_Error_syntax_error, MUD_TEXT("Expected comma"));
			goto bail;
		}
	}

bail:
	return first_child;
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
	p->at     = (char *)MUD_STRING_BYTES(source);
	p->end    = (char *)MUD_STRING_BYTES(source) + MUD_STRING_COUNT(source);
	p->line   = 1;

	mud_next_token(p);

	Mud_Parse_Result result = {0};
	result.error = Mud_Error_out_of_nodes;

	Mud_Node *root = mud_allocate_node(p);

	if (root != NULL)
	{
		result.error = Mud_Error_none;

		root->name  = MUD_TEXT("root");
		root->line  = 1;
		root->flags = Mud_Node_Flag_is_object|Mud_Node_Flag_has_name|Mud_Node_Flag_is_root;

		p->parent = root;

		root->first_child = mud_parse_list(p, 1, Mud_Token_eof);
	}

	result.node_count = p->out_nodes_used;
	MUD_STRING_BYTES_ASSIGN(result.error_message, p->message_buffer);
	MUD_STRING_COUNT_ASSIGN(result.error_message, p->message_length);
	return result;
}

#endif