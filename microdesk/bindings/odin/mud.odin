package mud

import "core:c"

when ODIN_OS == .Windows {
	foreign import mud_lib "lib/mud.lib"
} else {
	foreign import mud_lib "lib/libmud.a"
}

Bool :: b8

MESSAGE_BUFFER_SIZE :: 256

Token :: enum c.int {
	Eof        = -1,
	Invalid    = -2,
	String     = -3,
	Identifier = -4,
	Number     = -5,
	Comment    = -6,
}

Node_Flag :: enum u32 {
	Is_Array             = 0,
	Is_Object            = 1,
	Is_Tag               = 2,
	Has_Children         = 3,
	Has_Name             = 4,
	Has_Value            = 5,
	Is_Root              = 6,
	Is_Quoted_String     = 7,
	Is_Backticked_String = 8,
	Is_Identifier        = 9,
	Is_Number            = 10,
	Number_Is_Negative   = 11,
	Number_Is_Octal      = 12,
	Number_Is_Hex        = 13,
	Number_Is_Real       = 14,
	Number_Is_Scientific = 15,
	Is_True              = 16,
	Is_False             = 17,
	Is_Nil               = 18,
	Invalid              = 31,
}

Node_Flags :: bit_set[Node_Flag; u32]

IS_BOOL :: Node_Flags{.Is_True, .Is_False}

Error :: enum c.int {
	None         = 0,
	Syntax_Error = 1,
	Out_Of_Nodes = 2,
}

Node :: struct {
	parent:      ^Node,
	next:        ^Node,
	first_child: ^Node,
	first_tag:   ^Node,

	flags: Node_Flags,

	line: int,
	col:  int,

	name:             string,
	value:            string,
	value_unquoted:   string,
	leading_comment:  string,
	trailing_comment: string,
}

Parser :: struct {
	out_nodes:         [^]Node,
	out_nodes_used:    int,
	out_node_capacity: int,

	source: string,

	at:  [^]u8,
	end: [^]u8,

	parent: ^Node,

	line_start: [^]u8,
	line:       int,
	col:        int,

	insert_comma: Bool,

	leading_comment: ^Node,
	line_comment:    ^Node,

	token: struct {
		start:      [^]u8,
		end:        [^]u8,
		line_start: [^]u8,
		line:       int,
		col:        int,
		value:      string,
		is_newline: Bool,
		kind:       c.char,
		flags:      Node_Flags,
	},

	error:            Error,
	error_line_start: int,
	error_line:       int,
	error_col:        int,

	message_length: int,
	message_buffer: [MESSAGE_BUFFER_SIZE]c.char,
}

Parse_Result :: struct {
	root:             ^Node,
	error:            Error,
	error_message:    string,
	error_line_start: int,
	error_line:       int,
	error_col:        int,
	node_count:       int,
}

@(default_calling_convention = "c")
foreign mud_lib {
	@(link_name = "mud_parse_from_string")
	parse_from_string :: proc(parser: ^Parser, nodes_buffer: [^]Node, nodes_buffer_size: int, source: string) -> Parse_Result ---

	@(link_name = "mud_nil")
	nil_node :: proc() -> ^Node ---

	@(link_name = "mud_is_nil")
	is_nil :: proc(node: ^Node) -> Bool ---

	@(link_name = "mud_get_tag")
	get_tag :: proc(node: ^Node, name: string) -> ^Node ---

	@(link_name = "mud_has_tag")
	has_tag :: proc(node: ^Node, name: string) -> Bool ---

	@(link_name = "mud_get_child")
	get_child :: proc(node: ^Node, name: string) -> ^Node ---

	@(link_name = "mud_has_child")
	has_child :: proc(node: ^Node, name: string) -> Bool ---
}
