#define DC_IMPL
#define DC_STATIC
#include "dc.h"

#define MUD_STRING String
#define MUD_STRING_BYTES(str) ((char const *)(str).bytes)
#define MUD_STRING_BYTES_ASSIGN(str, value) (str).bytes = (char *)(value)
#define MUD_STRING_COUNT(str) ((Mud_Int)((str).count))
#define MUD_STRING_COUNT_ASSIGN(str, value) ((str).count = (isz)value)

#define MUD_IMPL
#define MUD_STATIC
#include "mud.h"

#include <stdio.h>

global Arena *arena;
global Mud_Node mud_nodes[1u << 16];

void mud_print(String_Builder *builder, Mud_Node *node, bool one_liner);

int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	arena = arena_make_default(S("arena"));

	String api_mud = string_from_memory(os_read_entire_file(arena, S("api.mud")));
	if (string_empty(api_mud))
	{
		fprintf(stderr, "Failed to open api.mud\n");
		return -1;
	}

	Mud_Parse_Result result = mud_parse_from_string(mud_nodes, ArrayCount(mud_nodes), api_mud);
	if (result.error != Mud_Error_none)
	{
		fprintf(stderr, "Mud Error (TODO: report properly)\n");
	}

	String_Builder sb;
	sb_init(&sb, arena);

	mud_print(&sb, mud_nodes, false);

	String str = sb_flatten(arena, &sb);
	printf("%.*s\n", Sx(str));

	return 0;
}



void mud_print(String_Builder *builder, Mud_Node *node, bool one_liner)
{
	for (Mud_Node *tag = node->first_tag; tag; tag = tag->next)
	{
		sb_appendf(builder, "@");
		mud_print(builder, tag, true);

		if (tag->flags & (Mud_Node_Flag_is_array|Mud_Node_Flag_is_object))
		{
			sb_appendc(builder, ' ');
		}
	}

	isz count = 0;

	if (node->flags & Mud_Node_Flag_has_name)
	{
		if (node->flags & Mud_Node_Flag_has_value)
		{
			sb_appendf(builder, "%cs = ", node->name);
		}
		else
		{
			sb_appendf(builder, "%cs ", node->name);
		}
	}

	if (node->flags & (Mud_Node_Flag_is_array|Mud_Node_Flag_is_object))
	{
		char *open  = (node->flags & Mud_Node_Flag_is_array) ? "[ " : "{ ";
		char *close = (node->flags & Mud_Node_Flag_is_array) ? "]" : "}";

		sb_appendf(builder, "%s", open);

		bool multiline = !one_liner && node->first_child;

		if (multiline) sb_newline(builder);
		for (Mud_Node *child = node->first_child; child; child = child->next)
		{
			if (count++ > 0)
			{
				if (one_liner)
				{
					sb_appends(builder, S(", "));
				}
				else
				{
					sb_appends(builder, S("\n"));
				}
			}

			if (multiline) sb_push_indent(builder, 1);

			if (multiline && !string_empty(child->leading_comment))
			{
				// lol
				count--;
				if (count++ > 0)
				{
					sb_newline(builder);
				}

				sb_append_reindented(builder, child->leading_comment, 0, 0);
				sb_newline(builder);
			}

			if (multiline) sb_append_line_indent(builder);
			mud_print(builder, child, one_liner);

			if (multiline && !string_empty(child->trailing_comment))
			{
				sb_appendf(builder, " %cs", child->trailing_comment);
			}

			if (multiline) sb_pop_indent(builder, 1);
		}
		if (multiline)
		{
			if (count > 0) sb_appends(builder, S("\n"));
			sb_append_line_indent(builder);
		}
		if (one_liner && count > 0) sb_appendc(builder, ' ');
		sb_appendf(builder, "%s", close);
	}
	else
	{
		sb_appendf(builder, "%cs", node->value);
	}
}