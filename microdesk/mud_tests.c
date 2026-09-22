#define DC_WITH_TESTING
#define DC_IMPL
#define DC_STATIC
#include <dc.h>

#define MUD_STRING String
#define MUD_STRING_BYTES(str) ((char const *)(str).chars)
#define MUD_STRING_BYTES_ASSIGN(str, value) (str).chars = (char *)(value)
#define MUD_STRING_COUNT(str) ((Mud_Int)((str).count))
#define MUD_STRING_COUNT_ASSIGN(str, value) ((str).count = (isz)value)

#define MUD_IMPL
#define MUD_STATIC
#include "mud.h"

global Arena     *arena;
global Mud_Node   mud_test_nodes[1u << 16];
global Mud_Parser mud_test_parser;

fn_local Mud_Parse_Result mud_test_parse(String source)
{
	return mud_parse_from_string(&mud_test_parser, mud_test_nodes, ArrayCount(mud_test_nodes), source);
}

fn_local isz mud_test_child_count(Mud_Node *node)
{
	isz result = 0;
	for (Mud_EachChild(child, node))
	{
		result += 1;
	}
	return result;
}

fn_local isz mud_test_tag_count(Mud_Node *node)
{
	isz result = 0;
	for (Mud_EachTag(tag, node))
	{
		result += 1;
	}
	return result;
}

fn_local Mud_Node *mud_test_child_at(Mud_Node *node, isz index)
{
	for (Mud_EachChild(child, node))
	{
		if (index == 0)
		{
			return child;
		}
		index -= 1;
	}
	return mud_nil();
}

void test_mud_document_structure(Test_Context *t)
{
	String source = S("value = 1\narray [ 1, 2 ]\nobject { a = 1, b = 2 }\n");

	Mud_Parse_Result result = mud_test_parse(source);
	TEST_CHECK(t, result.error == Mud_Error_none, "unexpected error %d", (int)result.error);
	TEST_CHECK(t, result.node_count > 0);

	Mud_Node *root = result.root;
	TEST_CHECK(t, !mud_is_nil(root));
	TEST_CHECK(t, string_match(root->name, S("root")));
	TEST_CHECK(t, !!(root->flags & Mud_Node_Flag_is_root));
	TEST_CHECK(t, !!(root->flags & Mud_Node_Flag_is_object));
	TEST_CHECK(t, !!(root->flags & Mud_Node_Flag_has_children));
	TEST_CHECK(t, mud_test_child_count(root) == 3);

	Mud_Node *value = mud_test_child_at(root, 0);
	TEST_CHECK(t, string_match(value->name, S("value")));
	TEST_CHECK(t, string_match(value->value, S("1")));
	TEST_CHECK(t, !!(value->flags & Mud_Node_Flag_has_name));
	TEST_CHECK(t, !!(value->flags & Mud_Node_Flag_has_value));
	TEST_CHECK(t, !(value->flags & Mud_Node_Flag_has_children));
	TEST_CHECK(t, value->parent == root);

	Mud_Node *array = mud_test_child_at(root, 1);
	TEST_CHECK(t, string_match(array->name, S("array")));
	TEST_CHECK(t, !!(array->flags & Mud_Node_Flag_is_array));
	TEST_CHECK(t, !!(array->flags & Mud_Node_Flag_has_children));
	TEST_CHECK(t, !(array->flags & Mud_Node_Flag_is_object));
	TEST_CHECK(t, mud_test_child_count(array) == 2);
	TEST_CHECK(t, string_match(mud_test_child_at(array, 0)->value, S("1")));
	TEST_CHECK(t, string_match(mud_test_child_at(array, 1)->value, S("2")));
	TEST_CHECK(t, !(mud_test_child_at(array, 0)->flags & Mud_Node_Flag_has_name));
	TEST_CHECK(t, mud_test_child_at(array, 0)->parent == array);

	Mud_Node *object = mud_test_child_at(root, 2);
	TEST_CHECK(t, string_match(object->name, S("object")));
	TEST_CHECK(t, !!(object->flags & Mud_Node_Flag_is_object));
	TEST_CHECK(t, !(object->flags & Mud_Node_Flag_is_array));
	TEST_CHECK(t, mud_test_child_count(object) == 2);
	TEST_CHECK(t, string_match(mud_test_child_at(object, 0)->name, S("a")));
	TEST_CHECK(t, string_match(mud_test_child_at(object, 1)->name, S("b")));

	result = mud_test_parse(S("array [ ]\nobject { }\n"));
	TEST_CHECK(t, result.error == Mud_Error_none);
	TEST_CHECK(t, mud_test_child_count(result.root) == 2);

	Mud_Node *empty_array = mud_test_child_at(result.root, 0);
	TEST_CHECK(t, !!(empty_array->flags & Mud_Node_Flag_is_array));
	TEST_CHECK(t, !(empty_array->flags & Mud_Node_Flag_has_children));
	TEST_CHECK(t, mud_test_child_count(empty_array) == 0);

	Mud_Node *empty_object = mud_test_child_at(result.root, 1);
	TEST_CHECK(t, !!(empty_object->flags & Mud_Node_Flag_is_object));
	TEST_CHECK(t, !(empty_object->flags & Mud_Node_Flag_has_children));
	TEST_CHECK(t, mud_test_child_count(empty_object) == 0);
}

void test_mud_nesting(Test_Context *t)
{
	String source = S("outer {\n\tmiddle {\n\t\tinner [ 1, 2, 3 ]\n\t}\n}\n");

	Mud_Parse_Result result = mud_test_parse(source);
	TEST_CHECK(t, result.error == Mud_Error_none);

	Mud_Node *outer  = mud_get_child(result.root, S("outer"));
	Mud_Node *middle = mud_get_child(outer, S("middle"));
	Mud_Node *inner  = mud_get_child(middle, S("inner"));

	TEST_CHECK(t, !mud_is_nil(outer));
	TEST_CHECK(t, !mud_is_nil(middle));
	TEST_CHECK(t, !mud_is_nil(inner));
	TEST_CHECK(t, mud_test_child_count(inner) == 3);
	TEST_CHECK(t, middle->parent == outer);
	TEST_CHECK(t, inner->parent == middle);
	TEST_CHECK(t, outer->parent == result.root);
}

void test_mud_scalar_values(Test_Context *t)
{
	String source = S(
		"plain    = 1\n"
		"negative = -2\n"
		"hex      = 0x1f\n"
		"octal    = 0777\n"
		"yes      = true\n"
		"no       = false\n"
		"nothing  = nil\n"
		"ident    = some_identifier\n"
		"quoted   = \"hello\"\n"
		"raw      = `raw text`\n");

	Mud_Parse_Result result = mud_test_parse(source);
	TEST_CHECK(t, result.error == Mud_Error_none);
	TEST_CHECK(t, mud_test_child_count(result.root) == 10);

	Mud_Node *node = mud_get_child(result.root, S("plain"));
	TEST_CHECK(t, !!(node->flags & Mud_Node_Flag_is_number));
	TEST_CHECK(t, !(node->flags & Mud_Node_Flag_number_is_negative));
	TEST_CHECK(t, string_match(node->value, S("1")));

	node = mud_get_child(result.root, S("negative"));
	TEST_CHECK(t, !!(node->flags & Mud_Node_Flag_is_number));
	TEST_CHECK(t, !!(node->flags & Mud_Node_Flag_number_is_negative));
	TEST_CHECK(t, string_match(node->value, S("-2")));

	node = mud_get_child(result.root, S("hex"));
	TEST_CHECK(t, !!(node->flags & Mud_Node_Flag_number_is_hex));
	TEST_CHECK(t, !(node->flags & Mud_Node_Flag_number_is_octal));

	node = mud_get_child(result.root, S("octal"));
	TEST_CHECK(t, !!(node->flags & Mud_Node_Flag_number_is_octal));
	TEST_CHECK(t, !(node->flags & Mud_Node_Flag_number_is_hex));

	node = mud_get_child(result.root, S("yes"));
	TEST_CHECK(t, !!(node->flags & Mud_Node_Flag_is_true));
	TEST_CHECK(t, !!(node->flags & Mud_Node_Flag_is_bool));
	TEST_CHECK(t, !(node->flags & Mud_Node_Flag_is_false));

	node = mud_get_child(result.root, S("no"));
	TEST_CHECK(t, !!(node->flags & Mud_Node_Flag_is_false));
	TEST_CHECK(t, !!(node->flags & Mud_Node_Flag_is_bool));
	TEST_CHECK(t, !(node->flags & Mud_Node_Flag_is_true));

	node = mud_get_child(result.root, S("nothing"));
	TEST_CHECK(t, !!(node->flags & Mud_Node_Flag_is_nil));

	node = mud_get_child(result.root, S("ident"));
	TEST_CHECK(t, !!(node->flags & Mud_Node_Flag_is_identifier));
	TEST_CHECK(t, !(node->flags & Mud_Node_Flag_is_number));
	TEST_CHECK(t, string_match(node->value, S("some_identifier")));

	node = mud_get_child(result.root, S("quoted"));
	TEST_CHECK(t, !!(node->flags & Mud_Node_Flag_is_quoted_string));
	TEST_CHECK(t, !(node->flags & Mud_Node_Flag_is_backticked_string));
	TEST_CHECK(t, string_match(node->value, S("\"hello\"")));
	TEST_CHECK(t, string_match(node->value_unquoted, S("hello")), "value_unquoted must strip both delimiters");

	node = mud_get_child(result.root, S("raw"));
	TEST_CHECK(t, !!(node->flags & Mud_Node_Flag_is_backticked_string));
	TEST_CHECK(t, !(node->flags & Mud_Node_Flag_is_quoted_string));
	TEST_CHECK(t, string_match(node->value, S("`raw text`")));
	TEST_CHECK(t, string_match(node->value_unquoted, S("raw text")), "value_unquoted must strip both delimiters");
}

void test_mud_delimiters(Test_Context *t)
{
	String commas   = S("object { a = 1, b = 2 }\narray [ 1, 2, 3 ]\n");
	String newlines = S("object {\n\ta = 1\n\tb = 2\n}\narray [\n\t1\n\t2\n\t3\n]\n");

	Mud_Parse_Result result = mud_test_parse(commas);
	TEST_CHECK(t, result.error == Mud_Error_none);
	isz comma_object_count = mud_test_child_count(mud_get_child(result.root, S("object")));
	isz comma_array_count  = mud_test_child_count(mud_get_child(result.root, S("array")));

	result = mud_test_parse(newlines);
	TEST_CHECK(t, result.error == Mud_Error_none);
	isz newline_object_count = mud_test_child_count(mud_get_child(result.root, S("object")));
	isz newline_array_count  = mud_test_child_count(mud_get_child(result.root, S("array")));

	TEST_CHECK(t, comma_object_count == 2);
	TEST_CHECK(t, comma_array_count == 3);
	TEST_CHECK(t, comma_object_count == newline_object_count, "commas and newlines must delimit alike");
	TEST_CHECK(t, comma_array_count == newline_array_count, "commas and newlines must delimit alike");
}

void test_mud_tags(Test_Context *t)
{
	String source = S("@first @second(x = 1, y = 2) thing [ 1, 2 ]\n@lonely other = 3\n");

	Mud_Parse_Result result = mud_test_parse(source);
	TEST_CHECK(t, result.error == Mud_Error_none);

	Mud_Node *thing = mud_get_child(result.root, S("thing"));
	TEST_CHECK(t, !mud_is_nil(thing));
	TEST_CHECK(t, mud_test_tag_count(thing) == 2);
	TEST_CHECK(t, mud_test_child_count(thing) == 2, "tags must not become children");

	TEST_CHECK(t, mud_has_tag(thing, S("first")));
	TEST_CHECK(t, mud_has_tag(thing, S("second")));
	TEST_CHECK(t, !mud_has_tag(thing, S("absent")));

	Mud_Node *first = mud_get_tag(thing, S("first"));
	TEST_CHECK(t, !mud_is_nil(first));
	TEST_CHECK(t, !!(first->flags & Mud_Node_Flag_is_tag));
	TEST_CHECK(t, string_match(first->name, S("first")));
	TEST_CHECK(t, mud_test_child_count(first) == 0);

	Mud_Node *second = mud_get_tag(thing, S("second"));
	TEST_CHECK(t, !mud_is_nil(second));
	TEST_CHECK(t, !!(second->flags & Mud_Node_Flag_is_tag));
	TEST_CHECK(t, mud_test_child_count(second) == 2, "tag arguments become children of the tag");
	TEST_CHECK(t, string_match(mud_get_child(second, S("x"))->value, S("1")));
	TEST_CHECK(t, string_match(mud_get_child(second, S("y"))->value, S("2")));

	TEST_CHECK(t, mud_is_nil(mud_get_tag(thing, S("absent"))));

	Mud_Node *other = mud_get_child(result.root, S("other"));
	TEST_CHECK(t, mud_has_tag(other, S("lonely")));
	TEST_CHECK(t, !mud_has_tag(other, S("first")), "tags must not leak between elements");
	TEST_CHECK(t, !mud_has_tag(result.root, S("first")));
}

void test_mud_comments(Test_Context *t)
{
	String source = S(
		"// Leading comment\n"
		"my_object {\n"
		"\tmember1 = value     // Trailing comment 1\n"
		"\tmember2 [ 1, 2, 3 ] // Trailing comment 2\n"
		"\t// Closing comment\n"
		"} // Trailing comment 3\n");

	Mud_Parse_Result result = mud_test_parse(source);
	TEST_CHECK(t, result.error == Mud_Error_none);

	Mud_Node *object = mud_get_child(result.root, S("my_object"));
	TEST_CHECK(t, !mud_is_nil(object));
	TEST_CHECK(t, string_match(object->leading_comment, S("// Leading comment")));
	TEST_CHECK(t, string_match(object->trailing_comment, S("// Trailing comment 3")));
	TEST_CHECK(t, string_match(object->closing_comment, S("\t// Closing comment")),
		"a lone comment keeps the leading whitespace of its line");

	Mud_Node *member1 = mud_get_child(object, S("member1"));
	Mud_Node *member2 = mud_get_child(object, S("member2"));
	TEST_CHECK(t, string_match(member1->trailing_comment, S("// Trailing comment 1")));
	TEST_CHECK(t, string_match(member2->trailing_comment, S("// Trailing comment 2")));
	TEST_CHECK(t, string_empty(member1->closing_comment));
	TEST_CHECK(t, string_empty(member2->closing_comment));

	result = mud_test_parse(S("// line one\n// line two\na = 1\n"));
	TEST_CHECK(t, result.error == Mud_Error_none);
	Mud_Node *node = mud_get_child(result.root, S("a"));
	TEST_CHECK(t, string_match(node->leading_comment, S("// line one\n// line two")),
		"adjacent lone comments form one block");

	result = mud_test_parse(S("a = 1\n// leads b, does not trail a\nb = 2\n"));
	TEST_CHECK(t, result.error == Mud_Error_none);
	Mud_Node *a = mud_get_child(result.root, S("a"));
	Mud_Node *b = mud_get_child(result.root, S("b"));
	TEST_CHECK(t, string_empty(a->trailing_comment), "a comment on its own line does not trail the element above");
	TEST_CHECK(t, string_match(b->leading_comment, S("// leads b, does not trail a")));
}

void test_mud_lookup(Test_Context *t)
{
	String source = S("alpha = 1\nbeta = 2\nnested { alpha = 3 }\n");

	Mud_Parse_Result result = mud_test_parse(source);
	TEST_CHECK(t, result.error == Mud_Error_none);

	TEST_CHECK(t, mud_has_child(result.root, S("alpha")));
	TEST_CHECK(t, mud_has_child(result.root, S("beta")));
	TEST_CHECK(t, !mud_has_child(result.root, S("gamma")));
	TEST_CHECK(t, !mud_has_child(result.root, S("alph")), "lookup must not match a prefix");
	TEST_CHECK(t, !mud_has_child(result.root, S("alphaa")));

	TEST_CHECK(t, string_match(mud_get_child(result.root, S("alpha"))->value, S("1")));
	TEST_CHECK(t, string_match(mud_get_child(result.root, S("beta"))->value, S("2")));

	Mud_Node *nested = mud_get_child(result.root, S("nested"));
	TEST_CHECK(t, string_match(mud_get_child(nested, S("alpha"))->value, S("3")),
		"lookup must not descend past direct children");
	TEST_CHECK(t, !mud_has_child(nested, S("beta")));
}

void test_mud_nil_nodes(Test_Context *t)
{
	Mud_Node *nil = mud_nil();

	TEST_CHECK(t, nil != NULL, "nil nodes are never null pointers");
	TEST_CHECK(t, mud_is_nil(nil));
	TEST_CHECK(t, mud_is_nil(NULL));
	TEST_CHECK(t, mud_is_nil(nil->parent));
	TEST_CHECK(t, mud_is_nil(nil->next));
	TEST_CHECK(t, mud_is_nil(nil->first_child));
	TEST_CHECK(t, mud_is_nil(nil->first_tag));

	Mud_Parse_Result result = mud_test_parse(S("a = 1\n"));
	TEST_CHECK(t, result.error == Mud_Error_none);
	TEST_CHECK(t, !mud_is_nil(result.root));

	Mud_Node *missing = mud_get_child(result.root, S("absent"));
	TEST_CHECK(t, mud_is_nil(missing));
	TEST_CHECK(t, missing == nil, "a missed lookup returns the shared nil node");
	TEST_CHECK(t, mud_is_nil(mud_get_tag(result.root, S("absent"))));

	TEST_CHECK(t, mud_is_nil(mud_get_child(nil, S("anything"))), "lookups on nil must be safe");
	TEST_CHECK(t, !mud_has_child(nil, S("anything")));
	TEST_CHECK(t, !mud_has_tag(nil, S("anything")));

	Mud_Node *last = mud_get_child(result.root, S("a"));
	TEST_CHECK(t, mud_is_nil(last->next), "the last sibling links to nil");
	TEST_CHECK(t, mud_is_nil(last->first_child));
	TEST_CHECK(t, mud_is_nil(last->first_tag));
}

void test_mud_errors(Test_Context *t)
{
	Mud_Parse_Result result = mud_test_parse(S("a = [ }"));
	TEST_CHECK(t, result.error == Mud_Error_syntax_error);
	TEST_CHECK(t, result.error_line == 1);
	TEST_CHECK(t, result.error_col == 6);
	TEST_CHECK(t, !string_empty(result.error_message));
	TEST_CHECK(t, string_match(result.error_message, S("Expected comma")));

	result = mud_test_parse(S("a = \"unterminated\n"));
	TEST_CHECK(t, result.error == Mud_Error_syntax_error, "an unterminated string is an error");
	TEST_CHECK(t, !string_empty(result.error_message));

	result = mud_test_parse(S("}\n"));
	TEST_CHECK(t, result.error == Mud_Error_syntax_error);
	TEST_CHECK(t, string_match(result.error_message, S("Expected identifier")));

	result = mud_test_parse(S("obj {\n\ta = 1\n"));
	TEST_CHECK(t, result.error == Mud_Error_syntax_error, "an unclosed object is an error");

	result = mud_test_parse(S("arr [ 1, 2\n"));
	TEST_CHECK(t, result.error == Mud_Error_syntax_error, "an unclosed array is an error");

	result = mud_parse_from_string(&mud_test_parser, mud_test_nodes, 0, S("a = 1\n"));
	TEST_CHECK(t, result.error == Mud_Error_out_of_nodes);
	TEST_CHECK(t, mud_is_nil(result.root));

	result = mud_test_parse(S("a = 1\nb = 2\n"));
	TEST_CHECK(t, result.error == Mud_Error_none, "a later good parse must clear the error state");
	TEST_CHECK(t, string_empty(result.error_message));
	TEST_CHECK(t, mud_test_child_count(result.root) == 2);
}

void test_mud_line_and_column(Test_Context *t)
{
	String source = S("first = 1\nsecond = 2\n\n  third = 3\n");

	Mud_Parse_Result result = mud_test_parse(source);
	TEST_CHECK(t, result.error == Mud_Error_none);

	TEST_CHECK(t, mud_get_child(result.root, S("first"))->line == 1);
	TEST_CHECK(t, mud_get_child(result.root, S("second"))->line == 2);
	TEST_CHECK(t, mud_get_child(result.root, S("third"))->line == 4, "blank lines still advance the line counter");
	TEST_CHECK(t, mud_get_child(result.root, S("third"))->col > mud_get_child(result.root, S("first"))->col,
		"indentation is reflected in the column");
}

void test_mud_real_document(Test_Context *t)
{
	String api_mud = string_from_memory(os_read_entire_file(arena, S("api.mud")));
	TEST_CHECK(t, !string_empty(api_mud), "api.mud must be readable from the working directory");

	if (string_empty(api_mud))
	{
		return;
	}

	Mud_Parse_Result result = mud_test_parse(api_mud);
	TEST_CHECK(t, result.error == Mud_Error_none, "api.mud must parse: (%zd:%zd) %.*s",
		result.error_line, result.error_col, Sx(result.error_message));
	TEST_CHECK(t, result.node_count > 1000);
	TEST_CHECK(t, result.node_count < ArrayCount(mud_test_nodes), "node buffer must not be exhausted");
	TEST_CHECK(t, mud_test_child_count(result.root) > 0);

	isz tagged = 0;
	for (Mud_EachChild(child, result.root))
	{
		if (!mud_is_nil(child->first_tag))
		{
			tagged += 1;
		}
	}
	TEST_CHECK(t, tagged > 0, "api.mud uses tags");

	TEST_CHECK(t, mud_has_child(result.root, S("Device_Address")));
	TEST_CHECK(t, mud_has_tag(mud_get_child(result.root, S("Device_Address")), S("alias")));
}

int entry_point(void)
{
	arena = arena_make(S("arena"));

	Test_Context *t = &(Test_Context){0};

	TEST_RUN(t, test_mud_document_structure);
	TEST_RUN(t, test_mud_nesting);
	TEST_RUN(t, test_mud_scalar_values);
	TEST_RUN(t, test_mud_delimiters);
	TEST_RUN(t, test_mud_tags);
	TEST_RUN(t, test_mud_comments);
	TEST_RUN(t, test_mud_lookup);
	TEST_RUN(t, test_mud_nil_nodes);
	TEST_RUN(t, test_mud_errors);
	TEST_RUN(t, test_mud_line_and_column);
	TEST_RUN(t, test_mud_real_document);

	return test_report(t);
}
